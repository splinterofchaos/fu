
#pragma once

#include <utility>      // forward, declval
#include <type_traits>  // is_member_object_pointer

namespace fu {

constexpr struct invoke_f {
  template<class F, class...X>
  constexpr auto operator() (F&& f, X&&...x) const
    -> decltype(std::declval<F>()(std::declval<X>()...))
  {
    return std::forward<F>(f)(std::forward<X>(x)...);
  }

  // Member function overloads:

  template<class F, class O, class...X>
  constexpr auto operator()(F f, O&& o, X&&...x) const
    -> decltype((std::declval<O>().*f)(std::declval<X>()...))
  {
    return (std::forward<O>(o).*f)(std::forward<X>(x)...);
  }

  template<class F, class O, class...X>
  constexpr auto operator() (F f, O&& o, X&&...x) const
    -> decltype((std::declval<O>()->*f)(std::declval<X>()...))
  {
    return (std::forward<O>(o)->*f)(std::forward<X>(x)...);
  }

  template<class F, class O>
  constexpr auto operator() (F f, O&& o) const
    -> std::enable_if_t< std::is_member_object_pointer<F>::value
                       , decltype(std::declval<O>().*f)>
  {
    return std::forward<O>(o).*f;
  }

  template<class F, class O>
  constexpr auto operator() (F f, O&& o) const
    -> std::enable_if_t< std::is_member_object_pointer<F>::value
                       , decltype(std::declval<O>()->*std::declval<F>())>
  {
    return std::forward<O>(o)->*f;
  }
} invoke{};

} // namespace fu
