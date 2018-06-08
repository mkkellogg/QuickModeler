#pragma once

#include "GestureAdapter.h"

#include "Core/Engine.h"
#include "Core/geometry/Vector3.h"

namespace Modeler {

    class OrbitControls {
    public:
        OrbitControls(std::weak_ptr<Core::Engine> engine, std::shared_ptr<Core::Camera> targetCamera);
        void handleGesture(GestureAdapter::GestureEvent event);

    private:
        Core::Point3r origin;
        std::weak_ptr<Core::Engine> engine;
        std::shared_ptr<Core::Camera> targetCamera;
    };

}
