#include <functional>

#include "GestureAdapter.h"

namespace Modeler {
    GestureAdapter::GestureAdapter(): mouseEventAdapter(std::bind(&GestureAdapter::onMouseEvent, this, std::placeholders::_1)) {

    }

    void GestureAdapter::setMouseAdapter(MouseAdapter& mouseAdapter) {
        mouseAdapter.setPipedEventAdapter(&this->mouseEventAdapter);
    }

    void GestureAdapter::onMouseEvent(MouseAdapter::MouseEvent event) {
        switch(event.getType()){
            case MouseAdapter::MouseEventType::ButtonDown:
                printf("button down! ");
            break;
        }
    }
}
