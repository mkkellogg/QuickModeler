#pragma once

#include <memory>

#include "MouseAdapter.h"

namespace Modeler {
    class GestureAdapter {
    public:
        GestureAdapter();

        void addMouseAdapter(const MouseAdapter& mouseAdapter);

    };
}
