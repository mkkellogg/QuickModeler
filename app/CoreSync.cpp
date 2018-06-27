#include "CoreSync.h"
#include "RenderSurface.h"

namespace Modeler {
    CoreSync::CoreSync(RenderSurface* renderSurface): renderSurface(renderSurface) {

    }

    CoreSync::~CoreSync() {

    }

    void CoreSync::run(Runnable runnable) {
        RendererGL::LifeCycleEventCallback temp = [this, runnable](RendererGL* renderer) {
            runnable(this->renderSurface->getRenderer().getEngine());
        };
        renderSurface->getRenderer().onUpdate(temp);
    }
}
