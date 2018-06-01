#pragma once

#include <memory>

#include "MouseAdapter.h"
#include "PipedEventAdapter.h"

namespace Modeler {
    class GestureAdapter {
    public:
        GestureAdapter();

        void setMouseAdapter(MouseAdapter& mouseAdapter);

    private:
        void onMouseEvent(MouseAdapter::MouseEvent event);

        PipedEventAdapter<MouseAdapter::MouseEvent> mouseEventAdapter;
    };
}
