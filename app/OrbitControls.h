#pragma once

#include "GestureAdapter.h"

#include "Core/Engine.h"
#include "Core/geometry/Vector3.h"

namespace Modeler {

    class OrbitControls {
    public:
        OrbitControls(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Camera> targetCamera);
        void handleGesture(GestureAdapter::GestureEvent event);

    private:
        Core::Point3r origin;
        Core::WeakPointer<Core::Engine> engine;
        Core::WeakPointer<Core::Camera> targetCamera;
    };

}
