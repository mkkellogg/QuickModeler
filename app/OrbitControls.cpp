
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

        Core::Vector3r viewStart(ndcStartX, ndcEndY, 0.0f);
        Core::Vector3r viewEnd(ndcEndX, ndcStartY, 0.0f);
        targetCamera->unProject(viewStart);
        targetCamera->unProject(viewEnd);

        Core::Matrix4x4 viewMat = targetCamera->getTransform().getWorldMatrix();
        viewMat.transform(viewStart, true);
        viewMat.transform(viewEnd, true);

        viewStart = Core::Point3r(viewStart.x, viewStart.y, viewStart.z) - this->origin;
        viewEnd = Core::Point3r(viewEnd.x, viewEnd.y, viewEnd.z) - this->origin;

        Core::Vector3r rotAxis;
        Core::Vector3r::cross(viewEnd, viewStart, rotAxis);
        rotAxis.normalize();

        Core::Point3r cameraPos;
        cameraPos.set(0, 0, 0);
        targetCamera->getTransform().transform(cameraPos, true);
        cameraPos.set(cameraPos.x - this->origin.x, cameraPos.y - this->origin.y, cameraPos.z - this->origin.z);
        Core::Real distanceFromOrigin = cameraPos.magnitude();

        using GesturePointer = GestureAdapter::GesturePointer;
        if (event.pointer == GesturePointer::Secondary) {

            Core::Real rotationScaleFactor = distanceFromOrigin * 0.1f;
            Core::Vector3r ndcDrag(ndcEndX - ndcStartX, ndcEndY - ndcStartY, 0.0f);
            float ndcDragLen = ndcDrag.magnitude();
            Core::Quaternion qA;
            qA.fromAngleAxis(ndcDragLen * rotationScaleFactor, rotAxis);
            Core::Matrix4x4 rot = qA.rotationMatrix();

            Core::Vector3r orgVec(this->origin.x, this->origin.y, this->origin.z);
            Core::Matrix4x4& localMatrix = targetCamera->getTransform().getLocalMatrix();
            orgVec.invert();
            localMatrix.preTranslate(orgVec);
            localMatrix.preMultiply(rot);
            orgVec.invert();
            localMatrix.preTranslate(orgVec);
            targetCamera->lookAt(this->origin);

        }
        else if (event.pointer == GesturePointer::Tertiary) {
            Core::Real translationScaleFactor = distanceFromOrigin * 0.5f;
            Core::Vector3r viewDragVector = viewEnd - viewStart;
            viewDragVector.invert();
            viewDragVector = viewDragVector * translationScaleFactor;
            this->origin = this->origin + viewDragVector;
            targetCamera->getTransform().getLocalMatrix().preTranslate(viewDragVector);
        }

    }
}
