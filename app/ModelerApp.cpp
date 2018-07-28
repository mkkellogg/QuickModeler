#include <memory>

#include <QGuiApplication>
#include <QtQuick/QQuickView>

#include "ModelerApp.h"
#include "RenderSurface.h"
#include "Util.h"

#include "Core/util/Time.h"
#include "Core/scene/Scene.h"
#include "Core/common/types.h"
#include "Core/math/Math.h"
#include "Core/math/Matrix4x4.h"
#include "Core/math/Quaternion.h"
#include "Core/render/Camera.h"
#include "Core/color/Color.h"
#include "Core/color/IntColor.h"
#include "Core/material/StandardAttributes.h"
#include "Core/geometry/Mesh.h"
#include "Core/render/RenderableContainer.h"
#include "Core/render/MeshRenderer.h"
#include "Core/render/RenderTarget.h"
#include "Core/material/BasicCubeMaterial.h"
#include "Core/material/BasicMaterial.h"
#include "Core/material/BasicLitMaterial.h"
#include "Core/material/StandardAttributes.h"
#include "Core/image/RawImage.h"
#include "Core/image/CubeTexture.h"
#include "Core/image/Texture2D.h"
#include "Core/util/WeakPointer.h"
#include "Core/asset/ModelLoader.h"
#include "Core/image/RawImage.h"
#include "Core/image/ImagePainter.h"
#include "Core/material/BasicTexturedMaterial.h"
#include "Core/geometry/GeometryUtils.h"
#include "Core/light/PointLight.h"
#include "Core/light/AmbientLight.h"
#include "Core/light/DirectionalLight.h"
#include "Core/scene/Transform.h"


using MeshContainer = Core::RenderableContainer<Core::Mesh>;

namespace Modeler {

    ModelerApp::ModelerApp(QObject *parent) : QObject(parent), engineReady(false),  orbitControls(nullptr), renderSurface(nullptr), coreSync(nullptr) {}

    void ModelerApp::initialize(QQuickView* rootView) {
        this->rootView = rootView;
        pipedGestureAdapter = std::make_shared<PipedEventAdapter<GestureAdapter::GestureEvent>>(std::bind(&ModelerApp::onGesture, this, std::placeholders::_1));
        for (unsigned int i = 0; i < MaxWindows; i++) this->liveWindows[i] = nullptr;
    }

    bool ModelerApp::addLoadedWindow(ModelerAppWindow* window, AppWindowType type) {
        if (this->liveWindows[(unsigned int)type] != nullptr) {
            qDebug() << "Window type (" << (int)type << ") already added!";
            return false;
        }

        if (!window->initialize(this)) {
            qDebug() << "Unable to initialize window!";
            return false;
        }

        this->liveWindows[(unsigned int)type] = window;

        if (type == AppWindowType::RenderSurface) {
            GestureAdapter* gestureAdapter = window->getGestureAdapter();
            if (gestureAdapter) {
                gestureAdapter->setPipedEventAdapter(pipedGestureAdapter);
            }

            RenderSurface* renderSurface = dynamic_cast<RenderSurface*>(window);
            if (renderSurface) {
                this->renderSurface = renderSurface;
                RendererGL::LifeCycleEventCallback initer = [this](RendererGL* renderer) {
                    this->engine = renderer->getEngine();
                    this->coreSync = std::make_shared<CoreSync>(this->renderSurface);
                    this->onEngineReady(engine);
                    this->orbitControls = std::make_shared<OrbitControls>(this->engine, this->renderCamera, this->coreSync);
                };
                renderSurface->getRenderer().onInit(initer);
            }
        }

        return true;
    }

    bool ModelerApp::addLoadedWindow(const std::string& windowName, AppWindowType type) {
        QObject *object = this->rootView->rootObject();
        QObject *rsObj = object->findChild<QObject*>(windowName.c_str());
        if (rsObj) {
            ModelerAppWindow* appWindow = dynamic_cast<Modeler::ModelerAppWindow*>(rsObj);
            if(appWindow) {
                return this->addLoadedWindow(appWindow, type);
            }
            else {
                qDebug() << "Unable to locate instance of app window!";
                return false;
            }
        }
        return true;
    }

    void ModelerApp::loadModel(const QString& path) {
        if (this->engineReady) {
            std::string sPath = path.toStdString();
            std::string filePrefix("file://");
            std::string pathPrefix = sPath.substr(0, 7) ;
            if (pathPrefix == filePrefix) {
                sPath = sPath.substr(7);
            }

            CoreSync::Runnable runnable = [this, sPath](Core::WeakPointer<Core::Engine> engine) {
                Core::ModelLoader& modelLoader = engine->getModelLoader();
                Core::WeakPointer<Core::Object3D> object = modelLoader.loadModel(sPath, .05f, false, false, true);
                this->sceneRoot->addChild(object);
                object->getTransform().translate(0.0f, 0.0f, 0.0f, Core::Transform::TransformationSpace::World);
            };
            this->coreSync->run(runnable);
        }
    }

    void ModelerApp::onGesture(GestureAdapter::GestureEvent event) {
        if (this->engineReady) {
            GestureAdapter::GestureEventType eventType = event.getType();
            switch(eventType) {
                case GestureAdapter::GestureEventType::Drag:
                case GestureAdapter::GestureEventType::Scroll:
                    this->orbitControls->handleGesture((event));
                break;
            }
        }
    }

    void ModelerApp::onEngineReady(Core::WeakPointer<Core::Engine> engine) {
        this->engineReady = true;

        Core::WeakPointer<Core::Scene> scene(engine->createScene());
        engine->setActiveScene(scene);
        this->sceneRoot = scene->getRoot();

        // ====== initial camera setup ====================
        Core::WeakPointer<Core::Object3D> cameraObj = engine->createObject3D<Core::Object3D>();
        this->renderCamera = engine->createPerspectiveCamera(cameraObj, Core::Camera::DEFAULT_FOV, Core::Camera::DEFAULT_ASPECT_RATIO, 0.1f, 100);
        this->sceneRoot->addChild(cameraObj);

        Core::Quaternion qA;
        qA.fromAngleAxis(0.0, 0, 1, 0);
        Core::Matrix4x4 rotationMatrixA;
        qA.rotationMatrix(rotationMatrixA);

        Core::Matrix4x4 worldMatrix;
        worldMatrix.multiply(rotationMatrixA);
        worldMatrix.translate(0, 0, 12);
        worldMatrix.translate(0, 5, 0);

        cameraObj->getTransform().getLocalMatrix().copy(worldMatrix);
        cameraObj->getTransform().updateWorldMatrix();
        cameraObj->getTransform().lookAt(Core::Point3r(0, 0, 0));


        Core::UInt32 gridMeshSubdivisions = 10;
        Core::Real gridSize = 15.0f;
        Core::Real texToWorld = 5.0f;
        Core::Real gridCellWorldSize = 1.0f;
        Core::WeakPointer<Core::Mesh> gridPlane = Core::GeometryUtils::createGrid(gridSize, gridSize, gridMeshSubdivisions, gridMeshSubdivisions, texToWorld, texToWorld);

        Core::WeakPointer<MeshContainer> gridPlaneObj(engine->createObject3D<MeshContainer>());

        Core::TextureAttributes texAttributes;
        texAttributes.FilterMode = Core::TextureFilter::TriLinear;
        texAttributes.MipMapLevel = 4;
        texAttributes.WrapMode = Core::TextureWrap::Repeat;
        Core::WeakPointer<Core::Texture2D> texture = engine->getGraphicsSystem()->createTexture2D(texAttributes);

        Core::UInt32 gridTextureSize = 1024;
        std::shared_ptr<Core::RawImage> rawImage = std::make_shared<Core::RawImage>(gridTextureSize, gridTextureSize);
        rawImage->init();

        Core::ImagePainter texturePainter(rawImage);
        texturePainter.setDrawColor(Core::IntColor(255, 255, 255, 255));


        Core::Real gridCellTextureSize = gridCellWorldSize / texToWorld;
        Core::UInt32 gridLineWidth = 4;
        Core::UInt32 gridLineHalfWidth = gridLineWidth / 2;
        Core::UInt32 cellSize = (Core::UInt32)(gridCellTextureSize * (Core::Real)gridTextureSize);
        Core::UInt32 halfCellSize = cellSize / 2;

        for (Core::UInt32 x = halfCellSize; x < gridTextureSize; x += cellSize) {
            Core::UInt32 upper = x + gridLineHalfWidth - 1;
            if (gridLineHalfWidth == 0) upper = x + 1;
            for (Core::UInt32 w = x - gridLineHalfWidth; w < upper; w++) {
                texturePainter.drawVerticalLine(w, 0, gridTextureSize);
                texturePainter.drawHorizontalLine(0, w, gridTextureSize);
            }
        }

        texture->build(rawImage);

        Core::WeakPointer<GridMaterial> gridPlaneMaterial = engine->createMaterial<GridMaterial>();
        gridPlaneMaterial->setTexture(texture);
        gridPlaneMaterial->setBounds(Core::Vector4r(-7.011, -7.011, -6.986, -6.986));

        Core::WeakPointer<Core::MeshRenderer> gridPlaneRenderer(engine->createRenderer<Core::MeshRenderer>(gridPlaneMaterial, gridPlaneObj));
        gridPlaneObj->addRenderable(gridPlane);
      //  this->sceneRoot->addChild(gridPlaneObj);
        gridPlaneObj->getTransform().rotate(1, 0, 0, -Core::Math::PI / 2.0f);




















       Core::Real cubeVertexPositions[] = {
                  // back
                  -1.0, -1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0,
                  -1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0,
                  // left
                  -1.0, -1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0,
                  -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0,
                  // right
                  1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 1.0,
                  1.0, 1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                  // top
                  -1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
                  // bottom
                  -1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, 1.0,
                  -1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0,
                  // front
                  1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0
              };

        Core::Real cubeVertexNormals[] = {
                  // back
                  0.0, 0.0, -1.0, 0.0,  0.0, 0.0, -1.0, 0.0,  0.0, 0.0, -1.0, 0.0,
                   0.0, 0.0, -1.0, 0.0,  0.0, 0.0, -1.0, 0.0,  0.0, 0.0, -1.0, 0.0,
                  // left
                  -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0,
                  -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0,
                  // right
                  1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
                  1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
                  // top
                  0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                  0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                  // bottom
                  0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0,
                  0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0,
                  // front
                   0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 1.0, 0.0,
                  0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 1.0, 0.0,
              };


        Core::Color slabColor(0.5f, 0.5f, 0.5f, 1.0f);

              Core::Real cubeVertexColors[] = {
                  // back
                  0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f,
                  0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f,
                  // left
                  0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f,
                  0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f,
                  // right
                  0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f,
                  0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f,
                  // top
                  0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f,
                  0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f,
                  // bottom
                  0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f,
                  0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f,
                  // front
                  0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f,
                  0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f,
              };

              Core::WeakPointer<Core::BasicLitMaterial> cubeMaterial(engine->createMaterial<Core::BasicLitMaterial>());
              cubeMaterial->build();


              // ======= slab ===============
              Core::WeakPointer<Core::Mesh> slab(engine->createMesh(36, false));
              slab->init();
              slab->enableAttribute(Core::StandardAttribute::Position);
              Core::Bool positionInited = slab->initVertexPositions();
              ASSERT(positionInited, "Unable to initialize slab mesh vertex positions.");
              slab->getVertexPositions()->store(cubeVertexPositions);

              slab->enableAttribute(Core::StandardAttribute::Color);
              Core::Bool colorInited = slab->initVertexColors();
              ASSERT(colorInited, "Unable to initialize slab mesh vertex colors.");
              slab->getVertexColors()->store(cubeVertexColors);

              slab->enableAttribute(Core::StandardAttribute::Normal);
              Core::Bool normalInited = slab->initVertexNormals();
              ASSERT(normalInited, "Unable to initialize slab mesh vertex normals.");
              slab->getVertexNormals()->store(cubeVertexNormals);

              slab->enableAttribute(Core::StandardAttribute::FaceNormal);
              Core::Bool faceNormalInited = slab->initVertexFaceNormals();

              slab->calculateNormals(75.0f);


              Core::WeakPointer<MeshContainer> leftSlabObj(engine->createObject3D<MeshContainer>());
              Core::WeakPointer<Core::MeshRenderer> leftSlabRenderer(engine->createRenderer<Core::MeshRenderer>(cubeMaterial, leftSlabObj));
              leftSlabObj->addRenderable(slab);
             // sceneRoot->addChild(leftSlabObj);
              leftSlabObj->getTransform().getLocalMatrix().scale(1.0f, 15.0f, 15.0f);
              leftSlabObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(-15.0f, 0.0f, 0.0f));



              Core::WeakPointer<MeshContainer> bottomSlabObj(engine->createObject3D<MeshContainer>());
              Core::WeakPointer<Core::MeshRenderer> bottomSlabRenderer(engine->createRenderer<Core::MeshRenderer>(cubeMaterial, bottomSlabObj));
              bottomSlabObj->addRenderable(slab);
              sceneRoot->addChild(bottomSlabObj);
              bottomSlabObj->getTransform().getLocalMatrix().scale(15.0f, 1.0f, 15.0f);
              bottomSlabObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(0.0f, -0.48f, 0.0f));


              Core::WeakPointer<MeshContainer> rightSlabObj(engine->createObject3D<MeshContainer>());
              Core::WeakPointer<Core::MeshRenderer> rightSlabRenderer(engine->createRenderer<Core::MeshRenderer>(cubeMaterial, rightSlabObj));
              rightSlabObj->addRenderable(slab);
             // sceneRoot->addChild(rightSlabObj);
              rightSlabObj->getTransform().getLocalMatrix().scale(1.0f, 15.0f, 15.0f);
              rightSlabObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(15.0f, 0.0f, 0.0f));


              Core::WeakPointer<MeshContainer> frontSlabObj(engine->createObject3D<MeshContainer>());
              Core::WeakPointer<Core::MeshRenderer> frontSlabRenderer(engine->createRenderer<Core::MeshRenderer>(cubeMaterial, frontSlabObj));
              frontSlabObj->addRenderable(slab);
             // sceneRoot->addChild(frontSlabObj);
              frontSlabObj->getTransform().getLocalMatrix().scale(15.0f, 15.0f, 1.0f);
              frontSlabObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(0.0f, 0.0f, -15.0f));



              Core::WeakPointer<MeshContainer> centerCubeObj(engine->createObject3D<MeshContainer>());
              Core::WeakPointer<Core::MeshRenderer> centerCubeRenderer(engine->createRenderer<Core::MeshRenderer>(cubeMaterial, centerCubeObj));
              centerCubeObj->addRenderable(slab);
             // sceneRoot->addChild(centerCubeObj);

              centerCubeObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(0.0f, 5.0f, 0.0f));









              Core::WeakPointer<Core::Object3D> ambientLightObject = engine->createObject3D();
              this->sceneRoot->addChild(ambientLightObject);
              Core::WeakPointer<Core::AmbientLight> ambientLight = engine->createLight<Core::AmbientLight>(ambientLightObject);
              ambientLight->setColor(0.25f, 0.25f, 0.25f, 1.0f);

              Core::WeakPointer<Core::Object3D> pointLightObject = engine->createObject3D();
              this->sceneRoot->addChild(pointLightObject);
              Core::WeakPointer<Core::PointLight> pointLight = engine->createPointLight<Core::PointLight>(pointLightObject, true, 2048, 0.0, 0.3);
              pointLight->setColor(1.0f, 1.0f, 1.0f, 1.0f);
              pointLight->setRadius(10.0f);

              Core::WeakPointer<Core::Object3D> directionalLightObject = engine->createObject3D();
              this->sceneRoot->addChild(directionalLightObject);
              Core::WeakPointer<Core::DirectionalLight> directionalLight = engine->createDirectionalLight<Core::DirectionalLight>(directionalLightObject, 3, true, 4096, 0.0003, 0.0);
              directionalLight->setColor(1.0, 1.0, 1.0, 1.0f);
              directionalLightObject->getTransform().lookAt(Core::Point3r(1.0f, -1.0f, 1.0f));

              engine->onUpdate([this, pointLightObject]() {

                static Core::Real rotationAngle = 0.0;
                if (Core::WeakPointer<Core::Object3D>::isValid(pointLightObject)) {
                  rotationAngle += 0.6 * Core::Time::getDeltaTime();
                  if (rotationAngle >= Core::Math::TwoPI) rotationAngle -= Core::Math::TwoPI;

                  Core::Quaternion qA;
                  qA.fromAngleAxis(rotationAngle, 0, 1, 0);
                  Core::Matrix4x4 rotationMatrixA;
                  qA.rotationMatrix(rotationMatrixA);

                  Core::Quaternion qB;
                  qB.fromAngleAxis(-0.8, 1, 0, 0);
                  Core::Matrix4x4 rotationMatrixB;
                  qB.rotationMatrix(rotationMatrixB);

                  Core::Matrix4x4 worldMatrix;

                  worldMatrix.preTranslate(10.0f, 10.0f, 0.0f);
                  worldMatrix.preMultiply(rotationMatrixA);
                  //worldMatrix.multiply(rotationMatrixB);

                  Core::WeakPointer<Core::Object3D> lightObjectPtr = pointLightObject;
                  lightObjectPtr->getTransform().getLocalMatrix().copy(worldMatrix);

                  auto vp = Core::Engine::instance()->getGraphicsSystem()->getCurrentRenderTarget()->getViewport();
                 // std::cerr << vp.z << ", " << vp.w << std::endl;
                  this->renderCamera->setAspectRatioFromDimensions(vp.z, vp.w);
                }
              }, true);


    }
}
