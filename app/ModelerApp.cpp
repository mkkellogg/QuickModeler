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

    void ModelerApp::onEngineReady(std::weak_ptr<Core::Engine> engine) {
        this->engineReady = true;

        Core::ValidWeakPointer<Core::Engine> enginePtr(engine);

        std::weak_ptr<Core::Scene> scene = enginePtr->createScene();
        enginePtr->setActiveScene(scene);

        Core::ValidWeakPointer<Core::Scene> scenePtr(scene);
        Core::ValidWeakPointer<Core::Object3D> sceneRootPtr = Core::ValidWeakPointer<Core::Object3D>(scenePtr->getRoot());

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

        std::weak_ptr<Core::BasicMaterial> cubeMaterial = enginePtr->createMaterial<Core::BasicMaterial>();
        Core::ValidWeakPointer<Core::BasicMaterial> cubeMaterialPtr(cubeMaterial);
        cubeMaterialPtr->build();

        std::weak_ptr<Core::Mesh> bigCube = enginePtr->createMesh<Core::Mesh>(36, false);
        Core::ValidWeakPointer<Core::Mesh> bigCubePtr(bigCube);

        bigCubePtr->enableAttribute(Core::StandardAttributes::Position);
        Core::Bool positionInited = bigCubePtr->initVertexPositions(36);
        ASSERT(positionInited, "Unable to initialize big cube mesh vertex positions.");
        bigCubePtr->getVertexPositions()->store(cubeVertexPositions);

        bigCubePtr->enableAttribute(Core::StandardAttributes::Color);
        Core::Bool colorInited = bigCubePtr->initVertexColors(36);
        ASSERT(colorInited, "Unable to initialize big cube mesh vertex colors.");
        bigCubePtr->getVertexColors()->store(cubeVertexColors);

        std::weak_ptr<MeshContainer> bigCubeObj = enginePtr->createObject3D<MeshContainer>();
        Core::ValidWeakPointer<MeshContainer> bigCubeObjPtr = Core::ValidWeakPointer<MeshContainer>(bigCubeObj);
        std::weak_ptr<Core::MeshRenderer> bigCubeRenderer = enginePtr->createRenderer<Core::MeshRenderer>(cubeMaterial, bigCubeObj);

        bigCubeObjPtr->addRenderable(bigCube);
        sceneRootPtr->addObject(bigCubeObj);
        bigCubeObjPtr->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(0.0f, 1.01f, 0.0f));


        std::weak_ptr<Core::Mesh> smallCube = enginePtr->createMesh<Core::Mesh>(36, false);
        Core::ValidWeakPointer<Core::Mesh> smallCubePtr(smallCube);

        smallCubePtr->enableAttribute(Core::StandardAttributes::Position);
        positionInited = smallCubePtr->initVertexPositions(36);
        ASSERT(positionInited, "Unable to initialize small cube mesh vertex positions.");
        smallCubePtr->getVertexPositions()->store(cubeVertexPositions);

        smallCubePtr->enableAttribute(Core::StandardAttributes::Color);
        colorInited = smallCubePtr->initVertexColors(36);
        ASSERT(colorInited, "Unable to initialize small cube mesh vertex colors.");
        smallCubePtr->getVertexColors()->store(cubeVertexColors);

        std::weak_ptr<MeshContainer> smallCubeObj = enginePtr->createObject3D<MeshContainer>();
        Core::ValidWeakPointer<MeshContainer> smallCubeObjPtr = Core::ValidWeakPointer<MeshContainer>(smallCubeObj);
        std::weak_ptr<Core::MeshRenderer> smallCubeRenderer = enginePtr->createRenderer<Core::MeshRenderer>(cubeMaterial, smallCubeObj);
        smallCubeObjPtr->addRenderable(smallCube);
        sceneRootPtr->addObject(smallCubeObj);
        smallCubeObjPtr->getTransform().getLocalMatrix().scale(Core::Vector3r(0.5f, 0.5f, 0.5f));
        smallCubeObjPtr->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(5.0f, 0.52f, 0.0f));



        // ======= Setup Plane =================
        std::weak_ptr<Core::Mesh> planeMesh = enginePtr->createMesh<Core::Mesh>(6, false);
        Core::ValidWeakPointer<Core::Mesh> planeMeshPtr(planeMesh);

        Core::Real planeVertexPositions[] = {
            -7.0, 0.0, -7.0, 1.0, 7.0, 0.0, -7.0, 1.0, -7.0, 0.0, 7.0, 1.0,
            7.0, 0.0, -7.0, 1.0, -7.0, 0.0, 7.0, 1.0, 7.0, 0.0, 7.0, 1.0,
        };

        planeMeshPtr->enableAttribute(Core::StandardAttributes::Position);
        Core::Bool planePositionInited = planeMeshPtr->initVertexPositions(6);
        ASSERT(planePositionInited, "Unable to initialize plane mesh vertex positions.");
        planeMeshPtr->getVertexPositions()->store(planeVertexPositions);

        Core::Real planeVertexColors[] = {
            0.65f, 0.65f, 0.65f, 1.0f, 0.65f, 0.65f, 0.65f, 1.0f, 0.65f, 0.65f, 0.65f, 1.0f,
            0.65f, 0.65f, 0.65f, 1.0f, 0.65f, 0.65f, 0.65f, 1.0f, 0.65f, 0.65f, 0.65f, 1.0f,
        };

        planeMeshPtr->enableAttribute(Core::StandardAttributes::Color);
        Core::Bool planeColorInited = planeMeshPtr->initVertexColors(6);
        ASSERT(planeColorInited, "Unable to initialize plane mesh vertex colors.");
        planeMeshPtr->getVertexColors()->store(planeVertexColors);

        std::weak_ptr<Core::BasicMaterial> planeMaterial = enginePtr->createMaterial<Core::BasicMaterial>();
        Core::ValidWeakPointer<Core::BasicMaterial> planeMaterialPtr(planeMaterial);
        planeMaterialPtr->build();

        std::weak_ptr<MeshContainer> planeObj = enginePtr->createObject3D<MeshContainer>();
        Core::ValidWeakPointer<MeshContainer> planeObjPtr = Core::ValidWeakPointer<MeshContainer>(planeObj);
        std::weak_ptr<Core::MeshRenderer> planeRenderer = enginePtr->createRenderer<Core::MeshRenderer>(planeMaterial, planeObj);
        planeObjPtr->addRenderable(planeMesh);
        sceneRootPtr->addObject(planeObj);



        this->renderCamera = enginePtr->createCamera();
        sceneRootPtr->addObject(this->renderCamera);

        Core::Quaternion qA;
        qA.fromAngleAxis(0.0, 0, 1, 0);
        Core::Matrix4x4 rotationMatrixA;
        qA.rotationMatrix(rotationMatrixA);

        Core::Matrix4x4 worldMatrix;
        worldMatrix.multiply(rotationMatrixA);
        worldMatrix.translate(12, 0, 0);
        worldMatrix.translate(0, 7, 0);

        Core::ValidWeakPointer<Core::Camera> renderCameraPtr(this->renderCamera);
        renderCameraPtr->getTransform().getLocalMatrix().copy(worldMatrix);
        renderCameraPtr->getTransform().updateWorldMatrix();
        renderCameraPtr->lookAt(Core::Point3r(0, 0, 0));

        this->orbitControls = new OrbitControls(this->engine, this->renderCamera);
    }
}
