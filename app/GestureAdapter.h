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

        enum class GesturePointer {
            Primary = 1,
            Secondary = 2,
            Tertiary = 4,
            PrimaryDouble = 3,
        };

        class GestureEvent {
        public:
            GestureEvent(GestureEventType type): type(type) {}
            GestureEventType getType() {return  type;}
            GesturePointer pointer;
            Core::Vector2i start;
            Core::Vector2i end;
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
            Core::Vector2i startPosition;
            Core::Vector2i position;
        };

        void onMouseEvent(MouseAdapter::MouseEvent event);

        PipedEventAdapter<MouseAdapter::MouseEvent> mouseEventAdapter;
        PointerState pointerStates[MAX_POINTERS];

        const PipedEventAdapter<GestureEvent>* pipedEventAdapter;
    };
}
