#pragma once

#include <memory>

#include "MouseAdapter.h"
#include "PipedEventAdapter.h"

namespace Modeler {
    class GestureAdapter {
    public:

        enum class GestureEventType {
            Drag = 0,
        };

        class GestureEvent {
        public:
            GestureEvent(GestureEventType type): type(type) {}
            GestureEventType getType() {return  type;}
            unsigned int button;
            Core::Vector2u start;
            Core::Vector2u end;
        private:
            GestureEventType type;
        };

        GestureAdapter();

        void setMouseAdapter(MouseAdapter& mouseAdapter);
        bool setPipedEventAdapter(const PipedEventAdapter<GestureEvent>* adapter);

    private:
        static const unsigned int MAX_POINTERS = 5;

        class PointerState {
        public:
            bool active = false;
            Core::Vector2u startPosition;
            Core::Vector2u position;
        };

        void onMouseEvent(MouseAdapter::MouseEvent event);

        PipedEventAdapter<MouseAdapter::MouseEvent> mouseEventAdapter;
        PointerState pointerStates[MAX_POINTERS];

        const PipedEventAdapter<GestureEvent>* pipedEventAdapter;
    };
}
