#pragma once

#include <boost/mpl/vector.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/reverse.hpp>

#include "blackhole/config.hpp"

BLACKHOLE_BEG_NS

namespace aux {

namespace util {

namespace vector {

template<typename... Args>
struct from_variadic;

template<typename T, typename... Args>
struct from_variadic<T, Args...> {
    typedef typename boost::mpl::push_back<
        typename from_variadic<Args...>::type,
        T
    >::type type;
};

template<typename T>
struct from_variadic<T> {
    typedef boost::mpl::vector<T> type;
};

template<>
struct from_variadic<> {
    typedef boost::mpl::vector<> type;
};

} // namespace vector

template<class T1, class T2> struct metaholder {};

} // namespace util

} // namespace aux

BLACKHOLE_END_NS
