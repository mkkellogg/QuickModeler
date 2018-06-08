#include <functional>

#include "Core/util/ValidWeakPointer.h"

#include "GestureAdapter.h"
#include "Util.h"

namespace Modeler {
    GestureAdapter::GestureAdapter() {
        mouseEventAdapter = std::make_shared<PipedEventAdapter<MouseAdapter::MouseEvent>>(std::bind(&GestureAdapter::onMouseEvent, this, std::placeholders::_1));
    }

    void GestureAdapter::setMouseAdapter(MouseAdapter& mouseAdapter) {
        mouseAdapter.setPipedEventAdapter(this->mouseEventAdapter);
    }

    bool GestureAdapter::setPipedEventAdapter(std::weak_ptr<PipedEventAdapter<GestureEvent>> adapter) {
        if(this->pipedEventAdapter.expired()) {
            this->pipedEventAdapter = adapter;
            return true;
        }
        else {
            return false;
        }
    }

    void GestureAdapter::onMouseEvent(MouseAdapter::MouseEvent event) {

        unsigned int pointerIndex = event.buttons;
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
                    gestureEvent.pointer = (GesturePointer)pointerIndex;
                    if (!this->pipedEventAdapter.expired()) {
                        Core::ValidWeakPointer<PipedEventAdapter<GestureEvent>> adapterPtr(this->pipedEventAdapter);
                        adapterPtr->accept(gestureEvent);
                    }
                    pointerState.position = event.position;
                }
            break;
        }
    }
}
