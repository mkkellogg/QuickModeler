#pragma once

#include <type_traits>
#include <unordered_map>
#include <functional>
#include <vector>

#include "Event.h"
#include "Types.h"

namespace Modeler {

    template <typename T, typename Enable = void>
    class EventQueue;

    template <typename T>
    class EventQueue<T, enable_if_t<std::is_base_of<Event, T>::value>> {

    public:

        typedef std::function<void(T)> EventCallback;

        EventQueue() {}

        void on(EventCallback callback) {
            queue.push_back(callback);
        }

    private:
        std::vector<EventCallback> queue;

    };

}
