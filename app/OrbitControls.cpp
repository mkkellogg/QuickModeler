
#include <memory>

#include "Core/geometry/Vector3.h"
#include "Core/math/Matrix4x4.h"
#include "Core/math/Quaternion.h"
#include "Core/util/WeakPointer.h"
#include "Core/Graphics.h"
#include "Core/render/Camera.h"

#include "OrbitControls.h"

namespace Modeler {
    OrbitControls::OrbitControls(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Camera> targetCamera): engine(engine), targetCamera(targetCamera) {

    }

    void OrbitControls::handleGesture(GestureAdapter::GestureEvent event) {

        if (event.getType() == GestureAdapter::GestureEventType::Scroll) {
            Core::WeakPointer<Core::Object3D> cameraObjPtr = this->targetCamera->getOwner();

            Core::Vector3r cameraVec;
            cameraVec.set(0, 0, -1);
            cameraObjPtr->getTransform().transform(cameraVec, false);
            cameraVec = cameraVec * event.scrollDistance;

            cameraObjPtr->getTransform().getLocalMatrix().preTranslate(cameraVec);

        }
        else if (event.getType() == GestureAdapter::GestureEventType::Drag) {

            Core::WeakPointer<Core::Graphics> graphics(this->engine->getGraphicsSystem());
            Core::WeakPointer<Core::Renderer> rendererPtr(graphics->getRenderer());

            Core::Vector4u viewport = rendererPtr->getViewport();
            Core::Real ndcStartX = (Core::Real)event.start.x / viewport.z * 2.0f - 1.0f;
            Core::Real ndcStartY = (Core::Real)event.start.y / viewport.w * 2.0f - 1.0f;
            Core::Real ndcEndX = (Core::Real)event.end.x / viewport.z * 2.0f - 1.0f;
            Core::Real ndcEndY = (Core::Real)event.end.y / viewport.w * 2.0f - 1.0f;

            Core::Vector3r viewStart(ndcStartX, ndcEndY, 0.0f);
            Core::Vector3r viewEnd(ndcEndX, ndcStartY, 0.0f);
            this->targetCamera->unProject(viewStart);
            this->targetCamera->unProject(viewEnd);

            Core::WeakPointer<Core::Object3D> cameraObjPtr = this->targetCamera->getOwner();

            Core::Matrix4x4 viewMat = cameraObjPtr->getTransform().getWorldMatrix();
            viewMat.transform(viewStart, true);
            viewMat.transform(viewEnd, true);

            viewStart = Core::Point3r(viewStart.x, viewStart.y, viewStart.z) - this->origin;
            viewEnd = Core::Point3r(viewEnd.x, viewEnd.y, viewEnd.z) - this->origin;

            Core::Vector3r viewStartN = Core::Vector3r(viewStart.x, viewStart.y, viewStart.z);
            viewStartN.normalize();

            Core::Vector3r viewEndN = Core::Vector3r(viewEnd.x, viewEnd.y, viewEnd.z);
            viewEndN.normalize();

            Core::Real dot = Core::Vector3r::dot(viewStartN, viewEndN);
             Core::Real angle = 0.0f;
            if (dot < 1.0f && dot > -1.0f) {
                angle = Core::Math::ACos(dot);
            }
            else if (dot <= -1.0f) {
                angle = 180.0f;
            }

            Core::Vector3r rotAxis;
            Core::Vector3r::cross(viewEnd, viewStart, rotAxis);
            rotAxis.normalize();

            Core::Point3r cameraPos;
            cameraPos.set(0, 0, 0);
            cameraObjPtr->getTransform().transform(cameraPos, true);
            cameraPos.set(cameraPos.x - this->origin.x, cameraPos.y - this->origin.y, cameraPos.z - this->origin.z);
            Core::Real distanceFromOrigin = cameraPos.magnitude();

            using GesturePointer = GestureAdapter::GesturePointer;
            if (event.pointer == GesturePointer::Secondary) {

                Core::Real rotationScaleFactor = Core::Math::Max(distanceFromOrigin, 1.0f);
                Core::Quaternion qA;
                qA.fromAngleAxis(angle * rotationScaleFactor, rotAxis);
                Core::Matrix4x4 rot = qA.rotationMatrix();

                Core::Vector3r orgVec(this->origin.x, this->origin.y, this->origin.z);
                Core::Matrix4x4& localMatrix = cameraObjPtr->getTransform().getLocalMatrix();
                orgVec.invert();
                localMatrix.preTranslate(orgVec);
                localMatrix.preMultiply(rot);
                orgVec.invert();
                localMatrix.preTranslate(orgVec);
                cameraObjPtr->getTransform().lookAt(this->origin);

            }
            else if (event.pointer == GesturePointer::Tertiary) {
                Core::Real translationScaleFactor = distanceFromOrigin * 0.5f;
                Core::Vector3r viewDragVector = viewEnd - viewStart;
                viewDragVector.invert();
                viewDragVector = viewDragVector * translationScaleFactor;
                this->origin = this->origin + viewDragVector;
                cameraObjPtr->getTransform().getLocalMatrix().preTranslate(viewDragVector);
            }
        }

    }
}
