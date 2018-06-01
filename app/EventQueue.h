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

        typedef std::function<void()> EventCallback;

        EventQueue() {}

        //template <typename U, typename Enable = void>
        //void on(EventCallback callback);

        template <typename U>
        void on(EventCallback callback);

    private:
        unsigned int nextIndex = 0;
        std::unordered_map<unsigned int, unsigned int> typeIndexMap;
        std::unordered_map<unsigned int, std::vector<T>> eventQueues;

        template <typename U>
        unsigned int getTypeCode() {
            return (unsigned int)typeid(U).hash_code();
        }
    };

    template <typename T>
    template <typename U>
    void EventQueue<T, enable_if_t<std::is_base_of<Event, T>::value>>::on(EventQueue<T, enable_if_t<std::is_base_of<Event, T>::value>>::EventCallback callback) {
        /*unsigned int typeCode = this->getTypeCode<T>();
        unsigned int queueIndex;
        if (this->typeIndexMap.find(typeCode) == this->typeIndexMap.end()) {
           this->typeIndexMap[typeCode] = queueIndex = nextIndex;
           nextIndex++;
        }
        else {
           queueIndex = this->typeIndexMap[typeCode];
        }


        if (this->eventQueues.find(queueIndex) == this->eventQueues.end()) {
           this->eventQueues[queueIndex] = std::vector<T>();
        }
        std::vector<T>& queue = this->eventQueues[queueIndex];*/

    }
}
