#pragma once

#include <memory>

#include "Exception.h"

namespace Modeler {

    class Util {
    public:

        template <typename T>
        static std::shared_ptr<T> expectValidWeakPointer(std::weak_ptr<T> ptr) {
            std::shared_ptr<T> sharedPtr = ptr.lock();
            if (!sharedPtr) {
                throw new AssertionFailedException("Expected vallid weak pointer.");
            }
            return sharedPtr;
        }

    private:
        Util();
    };

}
