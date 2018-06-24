#pragma once

#include "GestureAdapter.h"
#include "Core/Engine.h"
#include "Core/geometry/Vector3.h"

namespace Modeler {

    // forward declarations
    class RenderSurface;

    class OrbitControls {
    public:
        OrbitControls(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Camera> targetCamera, RenderSurface* renderSurface);
        void handleGesture(GestureAdapter::GestureEvent event);

    private:
        Core::Point3r origin;
        Core::WeakPointer<Core::Engine> engine;
        Core::WeakPointer<Core::Camera> targetCamera;
        RenderSurface* renderSurface;
    };

}
