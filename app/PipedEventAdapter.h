#pragma once

#include <functional>

namespace Modeler {
    template<typename T>
    class PipedEventAdapter {
    public:
        PipedEventAdapter(std::function<void(T)> callback): callback(callback) {

        }

        virtual void accept(T event) const {
            this->callback(event);
        }

    private:
       mutable std::function<void(T)> callback;
    };
}
