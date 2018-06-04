
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

        viewStart = Core::Point3r(viewStart.x, viewStart.y, viewStart.z) - this->origin;
        viewEnd = Core::Point3r(viewEnd.x, viewEnd.y, viewEnd.z) - this->origin;

        Core::Vector3r dragVector = viewEnd - viewStart;

       // printf("[%f, %f, %f], [%f, %f, %f], [%f, %f, %f]\n", viewStart.x, viewStart.y, viewStart.z, viewEnd.x, viewEnd.y, viewEnd.z, dragVector.x, dragVector.y, dragVector.z);

        Core::Vector3r viewDragVector = viewEnd - viewStart;
        viewDragVector.invert();

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

       // printf("%d\n", event.pointer);
        if (event.pointer == 2) {
            Core::Vector3r trans(dragVector.x, dragVector.y, dragVector.z);
            trans.invert();
            Core::Matrix4x4& localMatrix = targetCamera->getTransform().getLocalMatrix();
            localMatrix.preTranslate((trans));
            localMatrix.preMultiply(rot);
            trans.invert();
            localMatrix.preTranslate(trans);
            targetCamera->lookAt(this->origin);
        }
        else if (event.pointer == 1) {
            this->origin = this->origin + viewDragVector;
            viewMat.invert();
            viewMat.transform(viewDragVector, false);
            targetCamera->getTransform().getLocalMatrix().preTranslate(viewDragVector);
        }

    }
}
