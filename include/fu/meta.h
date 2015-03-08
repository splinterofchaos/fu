
#pragma once

#include <utility>
#include <functional>

#include <fu/basic.h>
#include <fu/iseq.h>

namespace fu {

// Define these outside meta because they can be used anywhere.

template<class X, X x> using Integral = std::integral_constant<X, x>;
template<char c>         using Char  = Integral<char, c>;
template<int x>          using Int   = Integral<int, x>;
template<unsigned int x> using UInt  = Integral<unsigned int, x>;
template<std::size_t x>  using Size  = Integral<std::size_t, x>;

namespace meta {

template<template<class...>class F, class...X>
using Apply = F<X...>;

template<class F, class...X>
using ApplyT = Apply<F::template type, X...>;

template<class T>
using Type = typename T::type;

template<class T>
constexpr auto value() {
  return T::value;
}

template<template<class...>class T, class...X>
struct Part {
  template<class...Y>
  using type = T<X..., Y...>;
};

template<template<class...>class T, template<class>class U>
struct UCompose {
  // FIXME: This should be able to take more arguments, but type = T<U<X>,Y...>
  // does not compile when Y... is empty.
  template<class X>
  using type = T<U<X>>;
};

template<template<class...>class Binary,
         template<class...>class T, template<class...>class U = T>
struct BCompose {
  template<class X, class Y>
  using type = Binary<T<X>, U<Y>>;
};

} // namespace meta
} // namespace fu
