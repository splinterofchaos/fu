
#pragma once

#include <utility>      // forward, declval
#include <type_traits>  // is_member_object_pointer

namespace fu {

constexpr struct maybe_deref_f {
  template< class O
          , class = std::enable_if_t<!std::is_pointer<std::decay_t<O>>{}>>
  constexpr O&& operator() (O&& o) const { return std::forward<O>(o); }

  template< class O
          , class = std::enable_if_t<std::is_pointer<std::decay_t<O>>{}>>
  constexpr decltype(auto) operator() (O&& o) const {
    return *std::forward<O>(o);
  }
} maybe_deref{};

constexpr struct invoke_member_f {
  template<class F, class O, class...X
          , class = std::enable_if_t<!std::is_member_object_pointer<F>{}>>
  constexpr decltype(auto) operator()(F f, O&& o, X&&...x) const {
    return (maybe_deref(std::forward<O>(o)).*f)(std::forward<X>(x)...);
  }

  template<class F, class O
          , class = std::enable_if_t<std::is_member_object_pointer<F>{}>>
  constexpr decltype(auto) operator() (F f, O&& o) const {
    return maybe_deref(std::forward<O>(o)).*f;
  }
} invoke_member{};

constexpr struct invoke_f {
  template< class F, class...X
          , bool IsMem = std::is_member_pointer<std::decay_t<F>>{}
          , class = std::enable_if_t<!IsMem>>
  constexpr decltype(auto) operator() (F&& f, X&&...x) const
  {
    return std::forward<F>(f)(std::forward<X>(x)...);
  }

  // Member function overloads:

  template< class F, class...X
          , bool IsMem = std::is_member_pointer<F>{}
          , class = std::enable_if_t<IsMem>>
  constexpr decltype(auto) operator() (F f, X&&...x) const {
    return invoke_member(f, std::forward<X>(x)...);
  }
} invoke{};

} // namespace fu
