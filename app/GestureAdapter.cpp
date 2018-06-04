#include <functional>

#include "GestureAdapter.h"

namespace Modeler {
    GestureAdapter::GestureAdapter(): mouseEventAdapter(std::bind(&GestureAdapter::onMouseEvent, this, std::placeholders::_1)) {

    }

    void GestureAdapter::setMouseAdapter(MouseAdapter& mouseAdapter) {
        mouseAdapter.setPipedEventAdapter(&this->mouseEventAdapter);
    }

    bool GestureAdapter::setPipedEventAdapter(const PipedEventAdapter<GestureEvent>* adapter) {
        if(this->pipedEventAdapter == nullptr) {
            this->pipedEventAdapter = adapter;
            return true;
        }
        else {
            return false;
        }
    }

    void GestureAdapter::onMouseEvent(MouseAdapter::MouseEvent event) {

        unsigned int pointerIndex = 0;
        unsigned int buttons = event.buttons;
        while(buttons != 0) {
            pointerIndex++;
            buttons = buttons >> 1;
        }
        if (pointerIndex >= MAX_POINTERS) return;

        PointerState& pointerState = pointerStates[pointerIndex];
        switch(event.getType()){
            case MouseAdapter::MouseEventType::ButtonDown:
                    pointerState.active = true;
                    pointerState.startPosition = event.position;
                    pointerState.position = event.position;
            break;
            case MouseAdapter::MouseEventType::ButtonUp:
                    pointerState.active = false;
                    pointerState.position = event.position;
            break;
            case MouseAdapter::MouseEventType::MouseMoved:
                if (pointerState.active) {
                    GestureEvent gestureEvent(GestureEventType::Drag);
                    gestureEvent.start = pointerState.position;
                    gestureEvent.end =  event.position;
                    if (this->pipedEventAdapter) {
                        this->pipedEventAdapter->accept(gestureEvent);
                    }
                    pointerState.position = event.position;
                }
            break;
        }
    }
}
