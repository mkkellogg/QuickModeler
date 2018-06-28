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
#include "Core/material/BasicCubeMaterial.h"
#include "Core/material/BasicMaterial.h"
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
        this->renderCamera = engine->createCamera(cameraObj);
        this->sceneRoot->addChild(cameraObj);

        Core::Quaternion qA;
        qA.fromAngleAxis(0.0, 0, 1, 0);
        Core::Matrix4x4 rotationMatrixA;
        qA.rotationMatrix(rotationMatrixA);

        Core::Matrix4x4 worldMatrix;
        worldMatrix.multiply(rotationMatrixA);
        worldMatrix.translate(0, 0, 12);
        worldMatrix.translate(0, 7, 0);

        cameraObj->getTransform().getLocalMatrix().copy(worldMatrix);
        cameraObj->getTransform().updateWorldMatrix();
        cameraObj->getTransform().lookAt(Core::Point3r(0, 0, 0));





       /* Core::Real gridPlaneVertices[] = {
            -1.0, 0.0, -1.0, 1.0, 1.0, 0.0, -1.0, 1.0, -1.0, 0.0, 1.0, 1.0,
            1.0, 0.0, -1.0, 1.0, -1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0
        };

        Core::Real gridPlaneColors[] = {
            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
        };

        Core::Real gridPlaneUVs[] = {
            0.0, 1.0, 1.0, 1.0, 0.0, 0.0,
            1.0, 1.0, 0.0, 0.0, 1.0, 0.0
        };

        Core::WeakPointer<Core::Mesh> gridPlane(engine->createMesh(6, false));
        gridPlane->enableAttribute(Core::StandardAttribute::Position);
        Core::Bool positionInited = gridPlane->initVertexPositions();
        ASSERT(positionInited, "Unable to initialize grid plane mesh vertex positions.");
        gridPlane->getVertexPositions()->store(gridPlaneVertices);

        gridPlane->enableAttribute(Core::StandardAttribute::Color);
        Core::Bool colorInited = gridPlane->initVertexColors();
        ASSERT(colorInited, "Unable to initialize grid plane mesh vertex colors.");
        gridPlane->getVertexColors()->store(gridPlaneColors);

        gridPlane->enableAttribute(Core::StandardAttribute::UV0);
        Core::Bool uvsInited = gridPlane->initVertexUVs();
        ASSERT(uvsInited, "Unable to initialize grid plane mesh vertex uvs.");
        gridPlane->getVertexUVs()->store(gridPlaneUVs);*/

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
        Core::RawImage * rawImage = new Core::RawImage(gridTextureSize, gridTextureSize);
        rawImage->init();

        Core::ImagePainter texturePainter(rawImage);
        texturePainter.setDrawColor(Core::IntColor4(255, 255, 255, 255));


        Core::Real gridCellTextureSize = gridCellWorldSize / texToWorld;
        Core::UInt32 gridLineWidth = 2;
        Core::UInt32 gridLineHalfWidth = gridLineWidth / 2;
        Core::UInt32 cellSize = (Core::UInt32)(gridCellTextureSize * (Core::Real)gridTextureSize);
        Core::UInt32 halfCellSize = cellSize / 2;

        for (Core::UInt32 x = halfCellSize; x < gridTextureSize; x += cellSize) {
            Core::UInt32 upper = x + gridLineHalfWidth;
            if (gridLineHalfWidth == 0) upper++;
            for (Core::UInt32 w = x - gridLineHalfWidth; w < upper; w++) {
                texturePainter.drawVerticalLine(w, 0, gridTextureSize);
                texturePainter.drawHorizontalLine(0, w, gridTextureSize);
            }
        }

        texture->build(rawImage);

        Core::WeakPointer<Core::BasicTexturedMaterial> gridPlaneMaterial = engine->createMaterial<Core::BasicTexturedMaterial>();
        gridPlaneMaterial->setTexture(texture);

        Core::WeakPointer<Core::MeshRenderer> gridPlaneRenderer(engine->createRenderer<Core::MeshRenderer>(gridPlaneMaterial, gridPlaneObj));
        gridPlaneObj->addRenderable(gridPlane);
        this->sceneRoot->addChild(gridPlaneObj);
        gridPlaneObj->getTransform().rotate(1, 0, 0, -Core::Math::PI / 2.0f);
        //gridPlaneObj->getTransform().getLocalMatrix().scale(10.0f, 10.0f, 10.0f);


    }
}
