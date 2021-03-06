#pragma once

#include <type_traits>

#include "blackhole/config.hpp"

#include "blackhole/detail/config/platform/underlying.hpp"

BLACKHOLE_BEG_NS

namespace aux {

template<typename T, class = void>
struct underlying_type {
    typedef T type;
};

template<typename T>
struct underlying_type<T, typename std::enable_if<std::is_enum<T>::value>::type> {
#if defined(BLACKHOLE_HAS_FEATURE_UNDERLYING_TYPE)
    typedef typename std::underlying_type<T>::type type;
#else
    typedef int type;
#endif
};

} // namespace aux

BLACKHOLE_END_NS
