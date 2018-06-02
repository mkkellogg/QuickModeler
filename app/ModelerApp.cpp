#include <memory>

#include <QGuiApplication>
#include <QtQuick/QQuickView>

#include "ModelerApp.h"
#include "RenderSurface.h"

#include "Core/util/Time.h"
#include "Core/scene/Scene.h"
#include "Core/common/types.h"
#include "Core/math/Math.h"
#include "Core/math/Matrix4x4.h"
#include "Core/math/Quaternion.h"
#include "Core/render/Camera.h"
#include "Core/material/StandardAttributes.h"
#include "Core/geometry/Mesh.h"
#include "Core/render/RenderableContainer.h"
#include "Core/render/MeshRenderer.h"
#include "Core/material/BasicCubeMaterial.h"
#include "Core/material/BasicMaterial.h"
#include "Core/material/StandardAttributes.h"
#include "Core/image/RawImage.h"
#include "Core/image/CubeTexture.h"

namespace Modeler {
    ModelerApp::ModelerApp(QQuickView* rootView): rootView(rootView), engine(nullptr),
        pipedGestureAdapter(std::bind(&ModelerApp::onGesture, this, std::placeholders::_1)) {
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
                gestureAdapter->setPipedEventAdapter(&pipedGestureAdapter);
            }

            RenderSurface* renderSurface = dynamic_cast<RenderSurface*>(window);
            if (renderSurface) {
                RendererGL::OnInitCallback initer = [this](RendererGL* renderer) {
                    this->engine = &renderer->getEngine();
                    this->onEngineReady(*engine);
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
        if (this->engine) {
            GestureAdapter::GestureEventType eventType = event.getType();
            switch(eventType) {
                case GestureAdapter::GestureEventType::Drag:
                {
                    std::shared_ptr<Core::Renderer> renderer = this->engine->getRenderer();

                    Core::Vector4u viewport = renderer->getViewport();
                    Core::Real ndcStartX = (Core::Real)event.start.x / viewport.z * 2.0f - 1.0f;
                    Core::Real ndcStartY = (Core::Real)event.start.y / viewport.w * 2.0f - 1.0f;
                    Core::Real ndcEndX = (Core::Real)event.end.x / viewport.z * 2.0f - 1.0f;
                    Core::Real ndcEndY = (Core::Real)event.end.y / viewport.w * 2.0f - 1.0f;

                    Core::Vector3r ndcDrag(ndcEndX - ndcStartX, ndcEndY - ndcStartY, 0.0f);
                    float ndcDragLen = ndcDrag.magnitude();

                    Core::Vector3r viewStart(ndcStartX, ndcEndY, 0.0f);
                    Core::Vector3r viewEnd(ndcEndX, ndcStartY, 0.0f);
                    renderCamera->unProject(viewStart);
                    renderCamera->unProject(viewEnd);

                    Core::Matrix4x4 viewMat = renderCamera->getTransform().getWorldMatrix();
                    viewMat.transform(viewStart, true);
                    viewMat.transform(viewEnd, true);

                    Core::Vector3r dragVector = viewEnd - viewStart;

                    Core::Vector3r rotAxis;
                    Core::Vector3r::cross(viewEnd, viewStart, rotAxis);
                    rotAxis.normalize();

                    Core::Point3r cameraPos;
                    renderCamera->getTransform().transform(cameraPos, true);
                    float baseLen = cameraPos.magnitude();

                    Core::Point3r rotated = cameraPos + dragVector;
                    float rotatedLen = rotated.magnitude();

                    float scaleFactor = baseLen / rotatedLen;
                    rotated = rotated * scaleFactor;

                    Core::Vector3r camVec(cameraPos.x, cameraPos.y, cameraPos.z);
                    Core::Vector3r rotatedVec(rotated.x, rotated.y, rotated.z);
                    Core::Quaternion qA;
                    qA.fromAngleAxis(ndcDragLen * 2.0 , rotAxis);
                    Core::Matrix4x4 rot = qA.rotationMatrix();

                    renderCamera->getTransform().getLocalMatrix().preMultiply(rot);
                    renderCamera->lookAt(Core::Point3r(0, 0, 0));

                }
                break;
            }
        }
    }

    void ModelerApp::onEngineReady(Core::Engine& engine) {

        std::vector<std::shared_ptr<Core::RawImage>> skyboxImages;
        std::shared_ptr<Core::CubeTexture> skyboxTexture;
        std::shared_ptr<Core::BasicMaterial> skyboxMaterial;
        std::shared_ptr<Core::ImageLoader> imageLoader;
        std::shared_ptr<Core::AssetLoader> assetLoader;

        std::shared_ptr<Core::Scene> scene = std::make_shared<Core::Scene>();
        engine.setScene(scene);

        std::shared_ptr<Core::Mesh> skyboxMesh = std::make_shared<Core::Mesh>(36, false);
        Core::Real vertexPositions[] = {
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

        skyboxMesh->enableAttribute(Core::StandardAttributes::Position);
        Core::Bool positionInited = skyboxMesh->initVertexPositions(36);
        ASSERT(positionInited, "Unable to initialize skybox mesh vertex positions.");
        skyboxMesh->getVertexPositions()->store(vertexPositions);

        Core::Real vertexColors[] = {
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

        skyboxMesh->enableAttribute(Core::StandardAttributes::Color);
        Core::Bool colorInited = skyboxMesh->initVertexColors(36);
        ASSERT(colorInited, "Unable to initialize skybox mesh vertex colors.");
        skyboxMesh->getVertexColors()->store(vertexColors);


        skyboxMaterial = std::make_shared<Core::BasicMaterial>();
        skyboxMaterial->build();

        std::shared_ptr<Core::RenderableContainer<Core::Mesh>> skyboxObj = std::make_shared<Core::RenderableContainer<Core::Mesh>>();
        std::shared_ptr<Core::MeshRenderer> skyboxRenderer = std::make_shared<Core::MeshRenderer>(skyboxMaterial, skyboxObj);
        skyboxObj->addRenderable(skyboxMesh);
        skyboxObj->setRenderer(skyboxRenderer);
        scene->getRoot()->addObject(skyboxObj);

        renderCamera = std::make_shared<Core::Camera>();
        scene->getRoot()->addObject(renderCamera);

        Core::Quaternion qA;
        qA.fromAngleAxis(0.0, 0, 1, 0);
        Core::Matrix4x4 rotationMatrixA;
        qA.rotationMatrix(rotationMatrixA);

        Core::Matrix4x4 worldMatrix;
        worldMatrix.multiply(rotationMatrixA);
        worldMatrix.translate(12, 0, 0);
        worldMatrix.translate(0, 7, 0);
        renderCamera->getTransform().getLocalMatrix().copy(worldMatrix);
        renderCamera->getTransform().updateWorldMatrix();
        renderCamera->lookAt(Core::Point3r(0, 0, 0));
    }
}
