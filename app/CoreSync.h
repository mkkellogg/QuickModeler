#pragma once

#include <functional>
#include <vector>

#include <QMutex>

#include "Core/Engine.h"

namespace Modeler {

    // forward declarations
    class RenderSurface;

    class CoreSync final {
    public:
        typedef std::function<void(Core::WeakPointer<Core::Engine>)> Runnable;

        CoreSync(RenderSurface* renderSurface);
        ~CoreSync();
        void run(Runnable runnable);

    private:
        RenderSurface* renderSurface;
        QMutex sync;
    };

}
