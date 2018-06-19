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
#include "Core/material/StandardAttributes.h"
#include "Core/geometry/Mesh.h"
#include "Core/render/RenderableContainer.h"
#include "Core/render/MeshRenderer.h"
#include "Core/material/BasicCubeMaterial.h"
#include "Core/material/BasicMaterial.h"
#include "Core/material/StandardAttributes.h"
#include "Core/image/RawImage.h"
#include "Core/image/CubeTexture.h"
#include "Core/util/WeakPointer.h"

using MeshContainer = Core::RenderableContainer<Core::Mesh>;

namespace Modeler {
    ModelerApp::ModelerApp(QQuickView* rootView):  engineReady(false), rootView(rootView), orbitControls(nullptr) {
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
                RendererGL::OnInitCallback initer = [this](RendererGL* renderer) {
                    this->engine = renderer->getEngine();
                    this->onEngineReady(engine);
                };
                renderSurface->getRenderer()->onInit(initer);
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

    void ModelerApp::onGesture(GestureAdapter::GestureEvent event) {
        if (this->engineReady) {
            GestureAdapter::GestureEventType eventType = event.getType();
            switch(eventType) {
                case GestureAdapter::GestureEventType::Drag:
                {
                    this->orbitControls->handleGesture((event));
                }
                break;
            }
        }
    }

    void ModelerApp::onEngineReady(Core::WeakPointer<Core::Engine> engine) {
        this->engineReady = true;

        Core::WeakPointer<Core::Scene> scene(engine->createScene());
        engine->setActiveScene(scene);
        Core::WeakPointer<Core::Object3D> sceneRoot(scene->getRoot());


        // ======= cube data =================
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
            1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
            // bottom
            -1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, 1.0,
            -1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0,
            // front
            1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0
        };

        Core::Real cubeVertexColors[] = {
            // back
            1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0,
            1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0,
            // left
            1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0,
            1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0,
            // right
            1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0,
            1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0,
            // top
            1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0,
            1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0,
            // bottom
            1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0,
            1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0,
            // front
            1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0,
            1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0,
        };

        Core::WeakPointer<Core::BasicMaterial> cubeMaterial(engine->createMaterial<Core::BasicMaterial>());
        cubeMaterial->build();


        // ======= big cube ===============
        Core::WeakPointer<Core::Mesh> bigCube(engine->createMesh(36, false));
        bigCube->init();
        bigCube->enableAttribute(Core::StandardAttributes::Position);
        Core::Bool positionInited = bigCube->initVertexPositions(36);
        ASSERT(positionInited, "Unable to initialize big cube mesh vertex positions.");
        bigCube->getVertexPositions()->store(cubeVertexPositions);

        bigCube->enableAttribute(Core::StandardAttributes::Color);
        Core::Bool colorInited = bigCube->initVertexColors(36);
        ASSERT(colorInited, "Unable to initialize big cube mesh vertex colors.");
        bigCube->getVertexColors()->store(cubeVertexColors);

        Core::WeakPointer<MeshContainer> bigCubeObj(engine->createObject3D<MeshContainer>());

        Core::WeakPointer<Core::MeshRenderer> bigCubeRenderer(engine->createRenderer<Core::MeshRenderer>(cubeMaterial, bigCubeObj));
        bigCubeObj->addRenderable(bigCube);
        sceneRoot->addChild(bigCubeObj);
        bigCubeObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(0.0f, 1.01f, 0.0f));


        // ==== small cube ============
        Core::WeakPointer<Core::Mesh> smallCube(engine->createMesh(36, false));
        smallCube->init();
        smallCube->enableAttribute(Core::StandardAttributes::Position);
        positionInited = smallCube->initVertexPositions(36);
        ASSERT(positionInited, "Unable to initialize small cube mesh vertex positions.");
        smallCube->getVertexPositions()->store(cubeVertexPositions);

        smallCube->enableAttribute(Core::StandardAttributes::Color);
        colorInited = smallCube->initVertexColors(36);
        ASSERT(colorInited, "Unable to initialize small cube mesh vertex colors.");
        smallCube->getVertexColors()->store(cubeVertexColors);

        Core::WeakPointer<MeshContainer> smallCubeObj( engine->createObject3D<MeshContainer>());
        Core::WeakPointer<Core::MeshRenderer> smallCubeRenderer(engine->createRenderer<Core::MeshRenderer>(cubeMaterial, smallCubeObj));
        smallCubeObj->addRenderable(smallCube);
        sceneRoot->addChild(smallCubeObj);
        smallCubeObj->getTransform().getLocalMatrix().scale(Core::Vector3r(0.5f, 0.5f, 0.5f));
        smallCubeObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(5.0f, 0.52f, 0.0f));


        // ======= plane =================
        Core::WeakPointer<Core::Mesh> planeMesh(engine->createMesh(6, false));
        planeMesh->init();
        Core::Real planeVertexPositions[] = {
            -7.0, 0.0, -7.0, 1.0, 7.0, 0.0, -7.0, 1.0, -7.0, 0.0, 7.0, 1.0,
            7.0, 0.0, -7.0, 1.0, -7.0, 0.0, 7.0, 1.0, 7.0, 0.0, 7.0, 1.0,
        };

        planeMesh->enableAttribute(Core::StandardAttributes::Position);
        Core::Bool planePositionInited = planeMesh->initVertexPositions(6);
        ASSERT(planePositionInited, "Unable to initialize plane mesh vertex positions.");
        planeMesh->getVertexPositions()->store(planeVertexPositions);

        Core::Real planeVertexColors[] = {
            0.65f, 0.65f, 0.65f, 1.0f, 0.65f, 0.65f, 0.65f, 1.0f, 0.65f, 0.65f, 0.65f, 1.0f,
            0.65f, 0.65f, 0.65f, 1.0f, 0.65f, 0.65f, 0.65f, 1.0f, 0.65f, 0.65f, 0.65f, 1.0f,
        };

        planeMesh->enableAttribute(Core::StandardAttributes::Color);
        Core::Bool planeColorInited = planeMesh->initVertexColors(6);
        ASSERT(planeColorInited, "Unable to initialize plane mesh vertex colors.");
        planeMesh->getVertexColors()->store(planeVertexColors);

        Core::WeakPointer<Core::BasicMaterial> planeMaterial(engine->createMaterial<Core::BasicMaterial>());
        planeMaterial->build();

        Core::WeakPointer<MeshContainer> planeObj(engine->createObject3D<MeshContainer>());
        engine->createRenderer<Core::MeshRenderer>(planeMaterial, planeObj);
        planeObj->addRenderable(planeMesh);
        sceneRoot->addChild(planeObj);


        // ====== initial camera setup ====================
        Core::WeakPointer<Core::Object3D> cameraObj = engine->createObject3D<Core::Object3D>();
        this->renderCamera = engine->createCamera(cameraObj);
        sceneRoot->addChild(cameraObj);

        Core::Quaternion qA;
        qA.fromAngleAxis(0.0, 0, 1, 0);
        Core::Matrix4x4 rotationMatrixA;
        qA.rotationMatrix(rotationMatrixA);

        Core::Matrix4x4 worldMatrix;
        worldMatrix.multiply(rotationMatrixA);
        worldMatrix.translate(12, 0, 0);
        worldMatrix.translate(0, 7, 0);

        Core::WeakPointer<Core::Object3D> cameraObjPtr(cameraObj);
        cameraObjPtr->getTransform().getLocalMatrix().copy(worldMatrix);
        cameraObjPtr->getTransform().updateWorldMatrix();
        cameraObjPtr->getTransform().lookAt(Core::Point3r(0, 0, 0));

        this->orbitControls = new OrbitControls(this->engine, this->renderCamera);
    }
}
