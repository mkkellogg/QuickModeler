#pragma once

#include <type_traits>

namespace Modeler {
    template< bool B, class T = void >
    using enable_if_t = typename std::enable_if<B,T>::type;
}
