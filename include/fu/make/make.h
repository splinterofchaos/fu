
#pragma once

#include <functional>
#include <utility>

namespace fu {

/// A type-class maker.
/// Ex:
///   std::make_pair  <=> MakeT<std::pair>{};
///   std::make_tuple <=> MakeT<std::tuple>{};
template<template<class...> class T> struct MakeT {
  /// Performs basic type transformations.
  template<class X>
  struct Ty { using type = X; };

  /// Converts reference wrappers to references.
  template<class X>
  struct Ty<std::reference_wrapper<X>> { using type = X&; };

  template<class X>
  using Ty_t = typename Ty<std::decay_t<X>>::type;

  template<class ...X>
  constexpr auto operator() (X&& ...x) const {
    return T<Ty_t<X>...>(std::forward<X>(x)...);
  }
};

/// Like MakeT, but using references, like std::tie.
///
/// Ex: TieT<std::tuple>{} <=> std::tie
template<template<class...> class T> struct TieT {
  template<class ...X>
  constexpr auto operator() ( X& ...x ) const {
    return T<X&...>(x...);
  }
};

/// Like TieT, but using perfect forwarding.
///
/// Ex: ForwardT<std::tuple>{} <=> std::forward_as_tuple
template<template<class...> class T> struct ForwardT {
  template<class ...X>
  constexpr auto operator() ( X&& ...x ) const {
    return T<X...>(std::forward<X>(x)...);
  }
};

} // namespace fu
