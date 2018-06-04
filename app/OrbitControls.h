#pragma once

#include "GestureAdapter.h"

#include "Core/Engine.h"

namespace Modeler {

    class OrbitControls {
    public:
        OrbitControls(std::shared_ptr<Core::Engine> engine, std::shared_ptr<Core::Camera> targetCamera);
        void handleGesture(GestureAdapter::GestureEvent event);

    private:
        std::shared_ptr<Core::Engine> engine;
        std::shared_ptr<Core::Camera> targetCamera;
    };

}
