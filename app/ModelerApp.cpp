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
#include "Core/util/ValidWeakPointer.h"

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

    void ModelerApp::onEngineReady(std::weak_ptr<Core::Engine> engine) {
        this->engineReady = true;

        Core::ValidWeakPointer<Core::Engine> enginePtr(engine);

        std::weak_ptr<Core::Scene> scene = enginePtr->createScene();
        enginePtr->setActiveScene(scene);

        Core::ValidWeakPointer<Core::Scene> scenePtr(scene);

        // ======= Setup Cube =================
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

        std::shared_ptr<Core::BasicMaterial> cubeMaterial = std::make_shared<Core::BasicMaterial>();
        cubeMaterial->build();

        std::shared_ptr<Core::Mesh> bigCube = std::make_shared<Core::Mesh>(36, false);
        bigCube->enableAttribute(Core::StandardAttributes::Position);
        Core::Bool positionInited = bigCube->initVertexPositions(36);
        ASSERT(positionInited, "Unable to initialize big cube mesh vertex positions.");
        bigCube->getVertexPositions()->store(cubeVertexPositions);

        bigCube->enableAttribute(Core::StandardAttributes::Color);
        Core::Bool colorInited = bigCube->initVertexColors(36);
        ASSERT(colorInited, "Unable to initialize big cube mesh vertex colors.");
        bigCube->getVertexColors()->store(cubeVertexColors);

        std::shared_ptr<Core::RenderableContainer<Core::Mesh>> bigCubeObj = std::make_shared<Core::RenderableContainer<Core::Mesh>>();
        std::shared_ptr<Core::MeshRenderer> bigCubeRenderer = std::make_shared<Core::MeshRenderer>(cubeMaterial, bigCubeObj);
        bigCubeObj->addRenderable(bigCube);
        bigCubeObj->setRenderer(bigCubeRenderer);
        scenePtr->getRoot()->addObject(bigCubeObj);
        bigCubeObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(0.0f, 1.01f, 0.0f));



        std::shared_ptr<Core::Mesh> smallCube = std::make_shared<Core::Mesh>(36, false);
        smallCube->enableAttribute(Core::StandardAttributes::Position);
        positionInited = smallCube->initVertexPositions(36);
        ASSERT(positionInited, "Unable to initialize small cube mesh vertex positions.");
        smallCube->getVertexPositions()->store(cubeVertexPositions);

        smallCube->enableAttribute(Core::StandardAttributes::Color);
        colorInited = smallCube->initVertexColors(36);
        ASSERT(colorInited, "Unable to initialize small cube mesh vertex colors.");
        smallCube->getVertexColors()->store(cubeVertexColors);

        std::shared_ptr<Core::RenderableContainer<Core::Mesh>> smallCubeObj = std::make_shared<Core::RenderableContainer<Core::Mesh>>();
        std::shared_ptr<Core::MeshRenderer> smallCubeRenderer = std::make_shared<Core::MeshRenderer>(cubeMaterial, smallCubeObj);
        smallCubeObj->addRenderable(smallCube);
        smallCubeObj->setRenderer(smallCubeRenderer);
        scenePtr->getRoot()->addObject(smallCubeObj);
        smallCubeObj->getTransform().getLocalMatrix().scale(Core::Vector3r(0.5f, 0.5f, 0.5f));
        smallCubeObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(5.0f, 0.52f, 0.0f));

        // ======= Setup Plane =================

        std::shared_ptr<Core::Mesh> planeMesh = std::make_shared<Core::Mesh>(6, false);
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


        std::shared_ptr<Core::BasicMaterial> planeMaterial = std::make_shared<Core::BasicMaterial>();
        planeMaterial->build();

        std::shared_ptr<Core::RenderableContainer<Core::Mesh>> planeObj = std::make_shared<Core::RenderableContainer<Core::Mesh>>();
        std::shared_ptr<Core::MeshRenderer> planeRenderer = std::make_shared<Core::MeshRenderer>(planeMaterial, planeObj);
        planeObj->addRenderable(planeMesh);
        planeObj->setRenderer(planeRenderer);
        scenePtr->getRoot()->addObject(planeObj);



        std::shared_ptr<Core::Camera> sharedRenderCamera = std::make_shared<Core::Camera>();
        this->renderCamera = sharedRenderCamera;
        scenePtr->getRoot()->addObject(sharedRenderCamera);

        Core::Quaternion qA;
        qA.fromAngleAxis(0.0, 0, 1, 0);
        Core::Matrix4x4 rotationMatrixA;
        qA.rotationMatrix(rotationMatrixA);

        Core::Matrix4x4 worldMatrix;
        worldMatrix.multiply(rotationMatrixA);
        worldMatrix.translate(12, 0, 0);
        worldMatrix.translate(0, 7, 0);
        sharedRenderCamera->getTransform().getLocalMatrix().copy(worldMatrix);
        sharedRenderCamera->getTransform().updateWorldMatrix();
        sharedRenderCamera->lookAt(Core::Point3r(0, 0, 0));

        this->orbitControls = new OrbitControls(this->engine, this->renderCamera);
    }
}
