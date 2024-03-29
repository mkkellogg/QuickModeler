#include <memory>
#include <exception>

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
#include "Core/geometry/Vector2.h"
#include "Core/geometry/Vector3.h"
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
#include "Core/scene/TransformationSpace.h"


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

                    MouseAdapter* mouseAdapter = this->renderSurface->getMouseAdapter();
                    mouseAdapter->onMouseButtonPressed(std::bind(&ModelerApp::onMouseButtonAction, this, std::placeholders::_1,  std::placeholders::_2,  std::placeholders::_3, std::placeholders::_4));
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

    void ModelerApp::loadModel(const QString& path, const QString& scaleText, const QString& smoothingThresholdText, const bool zUp) {
        if (this->engineReady) {
            std::string sPath = path.toStdString();
            std::string filePrefix("file://");
            std::string pathPrefix = sPath.substr(0, 7) ;
            if (pathPrefix == filePrefix) {
                sPath = sPath.substr(7);
            }

            std::string _scaleText = scaleText.toStdString();
            float scale = 1.0f;
            try {
                scale = std::stof(_scaleText);
            }
            catch (const std::invalid_argument& ia) {
                scale = 1.0f;
            }

            std::string _smoothingThresholdText = smoothingThresholdText.toStdString();
            int smoothingThreshold = 80;
            try {
                smoothingThreshold = std::stoi(_smoothingThresholdText);
            }
            catch (const std::invalid_argument& ia) {
                smoothingThreshold = 80;
            }
            if (smoothingThreshold < 0 ) smoothingThreshold = 0;
            if (smoothingThreshold >= 90) smoothingThreshold = 90;

            CoreSync::Runnable runnable = [this, sPath, scale, smoothingThreshold, zUp](Core::WeakPointer<Core::Engine> engine) {
                Core::ModelLoader& modelLoader = engine->getModelLoader();
                Core::WeakPointer<Core::Object3D> rootObject = modelLoader.loadModel(sPath, scale, smoothingThreshold, false, false, true);
                this->sceneRoot->addChild(rootObject);

                Core::WeakPointer<Core::Scene> scene = engine->getActiveScene();
                scene->visitScene(rootObject, [this, &rootObject](Core::WeakPointer<Core::Object3D> obj){
                    Core::WeakPointer<Core::RenderableContainer<Core::Mesh>> meshContainer =
                            Core::WeakPointer<Core::Object3D>::dynamicPointerCast<Core::RenderableContainer<Core::Mesh>>(obj);
                    if (meshContainer) {
                        std::vector<Core::WeakPointer<Core::Mesh>> meshes = meshContainer->getRenderables();
                        for (Core::WeakPointer<Core::Mesh> mesh : meshes) {
                            this->rayCaster.addObject(obj, mesh);
                            this->meshToObjectMap[mesh->getObjectID()] = obj;
                        }
                    }
                });

                if (zUp) {
                    rootObject->getTransform().rotate(1.0f, 0.0f, 0.0f, -Core::Math::PI / 2.0);
                }
            };
            this->coreSync->run(runnable);
        }
    }

    void ModelerApp::onMouseButtonAction(MouseAdapter::MouseEventType type, Core::UInt32 button, Core::UInt32 x, Core::UInt32 y) {
        switch(type) {
            case MouseAdapter::MouseEventType::ButtonPress:
            {
                Core::Point3r pos((Core::Real)x, (Core::Real)y, (Core::Real)-1.0f);
                if (button == 1) {
                    CoreSync::Runnable runnable = [this, pos](Core::WeakPointer<Core::Engine> engine) {

                        Core::WeakPointer<Core::Graphics> graphics = this->engine->getGraphicsSystem();
                        Core::WeakPointer<Core::Renderer> rendererPtr = graphics->getRenderer();
                        Core::Vector4u viewport = graphics->getViewport();

                        Core::Real ndcX = (Core::Real)pos.x / (Core::Real)viewport.z * 2.0f - 1.0f;
                        Core::Real ndcY = -((Core::Real)pos.y / (Core::Real)viewport.w * 2.0f - 1.0f);
                        Core::Point3r ndcPos(ndcX, ndcY, -1.0);
                        this->renderCamera->unProject(ndcPos);
                        Core::Transform& camTransform = this->renderCamera->getOwner()->getTransform();
                        camTransform.updateWorldMatrix();
                        Core::Matrix4x4 camMat = camTransform.getWorldMatrix();
                        Core::Matrix4x4 camMatInverse = camMat;
                        camMatInverse.invert();

                        Core::Point3r worldPos = ndcPos;
                        camMat.transform(worldPos);
                        Core::Point3r origin;
                        camMat.transform(origin);
                        Core::Vector3r rayDir = worldPos - origin;
                        rayDir.normalize();
                        Core::Ray ray(origin, rayDir);

                        std::vector<Core::Hit> hits;
                        Core::Bool hit = this->rayCaster.castRay(ray, hits);

                       // std::cerr << "Hit count: " << hits.size() << std::endl;
                        if (hits.size() > 0) {
                            Core::Hit& hit = hits[0];
                            Core::WeakPointer<Core::Mesh> hitObject = hit.Object;
                            Core::WeakPointer<Core::Object3D> rootObject =this->meshToObjectMap[hitObject->getObjectID()];
                            this->selectedObject = rootObject;
                            if (this->selectedObject) {
                                // std::cerr << "Selected: " << this->selectedObject->getObjectID() << std::endl;
                            }
                        }

                    };
                    if (this->coreSync) {
                        this->coreSync->run(runnable);
                    }
                }

                break;
            }
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


        // ====== model platform vertex attributes ====================
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

        Core::Color slabColor(0.0f, 0.53f, 0.16f, 1.0f);
        Core::Real cubeVertexColors[] = {
            // back
            slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
            slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
            // left
            slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
            slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
            // right
            slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
            slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
            // top
            slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
            slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
            // bottom
            slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
            slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
            // front
            slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
            slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0
        };

        Core::WeakPointer<Core::BasicLitMaterial> cubeMaterial(engine->createMaterial<Core::BasicLitMaterial>());
        cubeMaterial->build();


        // ======= model platform objects ===============
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

        slab->calculateBoundingBox();
        slab->calculateNormals(75.0f);

        Core::WeakPointer<MeshContainer> bottomSlabObj(engine->createObject3D<MeshContainer>());
        Core::WeakPointer<Core::MeshRenderer> bottomSlabRenderer(engine->createRenderer<Core::MeshRenderer>(cubeMaterial, bottomSlabObj));
        bottomSlabObj->addRenderable(slab);
        sceneRoot->addChild(bottomSlabObj);
        this->rayCaster.addObject(bottomSlabObj, slab);
        this->meshToObjectMap[slab->getObjectID()] = bottomSlabObj;
        // this->meshToObjectMap[slab->getObjectID()] = Core::WeakPointer<MeshContainer>::dynamicPointerCast<Core::Object3D>( bottomSlabObj);
        bottomSlabObj->getTransform().getLocalMatrix().scale(15.0f, 1.0f, 15.0f);
        bottomSlabObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(0.0f, -1.0f, 0.0f));
        bottomSlabObj->getTransform().getLocalMatrix().preRotate(0.0f, 1.0f, 0.0f,Core::Math::PI / 4.0f);


        // ========== lights ============================
        Core::WeakPointer<Core::Object3D> ambientLightObject = engine->createObject3D();
        this->sceneRoot->addChild(ambientLightObject);
        Core::WeakPointer<Core::AmbientLight> ambientLight = engine->createLight<Core::AmbientLight>(ambientLightObject);
        ambientLight->setColor(0.25f, 0.25f, 0.25f, 1.0f);

        Core::WeakPointer<Core::Object3D> pointLightObject = engine->createObject3D();
        this->sceneRoot->addChild(pointLightObject);
        Core::WeakPointer<Core::PointLight> pointLight = engine->createPointLight<Core::PointLight>(pointLightObject, true, 2048, 0.0115, 0.35);
        pointLight->setColor(1.0f, 1.0f, 1.0f, 1.0f);
        pointLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
        pointLight->setRadius(10.0f);

        Core::WeakPointer<Core::Object3D> directionalLightObject = engine->createObject3D();
        this->sceneRoot->addChild(directionalLightObject);
        Core::WeakPointer<Core::DirectionalLight> directionalLight = engine->createDirectionalLight<Core::DirectionalLight>(directionalLightObject, 3, true, 4096, 0.0001, 0.0005);
        directionalLight->setColor(1.0, 1.0, 1.0, 1.0f);
        directionalLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
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
                this->renderCamera->setAspectRatioFromDimensions(vp.z, vp.w);

            }
        }, true);



        this->highlightMaterial = engine->createMaterial<Core::BasicColoredMaterial>();
        this->highlightMaterial->setBlendingMode(Core::RenderState::BlendingMode::Custom);
        this->highlightMaterial->setSourceBlendingMethod(Core::RenderState::BlendingMethod::SrcAlpha);
        this->highlightMaterial->setDestBlendingMethod(Core::RenderState::BlendingMethod::OneMinusSrcAlpha);
        this->highlightMaterial->setLit(false);
        engine->onRender([this]() {
             Core::Color highlightColor(1.0, 0.65, 0.0, 0.5);
             Core::Color highlightLineColor(1.0, 0.65, 0.0, 1.0);
             if (this->selectedObject) {
                  //  std::cerr << "rendering..." << this->selectedObject->getObjectID() << std::endl;
                    this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Color, false);
                    this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Depth, false);

                    this->highlightMaterial->setZOffset(-.00005f);
                    this->highlightMaterial->setColor(highlightColor);
                    Core::Engine::instance()->getGraphicsSystem()->getRenderer()->renderObjectBasic(this->selectedObject, this->renderCamera, this->highlightMaterial);
                    this->highlightMaterial->setRenderStyle(Core::RenderStyle::Line);
                    this->highlightMaterial->setZOffset(-.0001f);
                    this->highlightMaterial->setColor(highlightLineColor);
                    Core::Engine::instance()->getGraphicsSystem()->getRenderer()->renderObjectBasic(this->selectedObject, this->renderCamera, this->highlightMaterial);
                    this->highlightMaterial->setRenderStyle(Core::RenderStyle::Fill);
                    this->highlightMaterial->setColor(highlightColor);

                    this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Color, true);
                    this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Depth, true);
            }
        }, true);
    }
}
