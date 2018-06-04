
#include <memory>

#include "OrbitControls.h"

#include "Core/geometry/Vector3.h"
#include "Core/math/Matrix4x4.h"
#include "Core/math/Quaternion.h"

namespace Modeler {
    OrbitControls::OrbitControls(std::shared_ptr<Core::Engine> engine, std::shared_ptr<Core::Camera> targetCamera): engine(engine), targetCamera(targetCamera) {

    }

    void OrbitControls::handleGesture(GestureAdapter::GestureEvent event) {
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
        targetCamera->unProject(viewStart);
        targetCamera->unProject(viewEnd);

        Core::Matrix4x4 viewMat = targetCamera->getTransform().getWorldMatrix();
        viewMat.transform(viewStart, true);
        viewMat.transform(viewEnd, true);

        Core::Vector3r dragVector = viewEnd - viewStart;

        Core::Vector3r rotAxis;
        Core::Vector3r::cross(viewEnd, viewStart, rotAxis);
        rotAxis.normalize();

        Core::Point3r cameraPos;
        targetCamera->getTransform().transform(cameraPos, true);
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

        targetCamera->getTransform().getLocalMatrix().preMultiply(rot);
        targetCamera->lookAt(Core::Point3r(0, 0, 0));

    }
}
