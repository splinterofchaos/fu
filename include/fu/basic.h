
#pragma once

#include <utility>
#include <functional>

#include <fu/iseq.h>

/// This file includes the basic functionality used to build other modules.

namespace fu {

template<bool b, class T = void>
using enable_if_t = typename std::enable_if<b, T>::type;

/// identity(x) = x
/// identity(f, x...) = f(x...)
constexpr struct identity_f {
  template<class X>
  constexpr X operator() (X&& x) const {
    return std::forward<X>(x);
  }
} identity{};

constexpr struct invoke_f {
  template<class F, class...X>
  constexpr auto operator() (F&& f, X&&...x) const
    -> decltype(std::declval<F>()(std::declval<X>()...))
  {
    return std::forward<F>(f)(std::forward<X>(x)...);
  }

  // Member function overloads:

  template<class F, class O, class...X>
  constexpr auto operator()(F&& f, O&& o, X&&...x) const
    -> decltype((std::declval<O>().*f)(std::declval<X>()...))
  {
    return (std::forward<O>(o).*f)(std::forward<X>(x)...);
  }

  template<class F, class O, class...X>
  constexpr auto operator() (F&& f, O&& o, X&&...x) const
    -> decltype((std::declval<O>()->*f)(std::declval<X>()...))
  {
    return (std::forward<O>(o)->*f)(std::forward<X>(x)...);
  }

  template<class F, class O>
  constexpr auto operator() (F&& f, O&& o) const
    -> enable_if_t< std::is_member_object_pointer<F>::value
                  , decltype(std::declval<O>().*f)>
  {
    return std::forward<O>(o).*f;
  }

  template<class F, class O>
  constexpr auto operator() (F&& f, O&& o) const
    -> enable_if_t< std::is_member_object_pointer<F>::value
                  , decltype(std::declval<O>()->*std::declval<F>())>
  {
    return std::forward<O>(o)->*std::forward<F>(f);
  }
} invoke{};

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

  template<class ...X, class R = T<Ty_t<X>...>>
  constexpr R operator() (X&& ...x) const {
    return R(std::forward<X>(x)...);
  }
};

/// Like MakeT, but using references, like std::tie.
///
/// Ex: TieT<std::tuple>{} <=> std::tie
template<template<class...> class T> struct TieT {
  template<class ...X, class R = T<X&...>>
  constexpr R operator() ( X& ...x ) const {
    return R(x...);
  }
};

/// Like TieT, but using perfect forwarding.
///
/// Ex: ForwardT<std::tuple>{} <=> std::forward_as_tuple
template<template<class...> class T> struct ForwardT {
  template<class ...X>
  constexpr T<X...> operator() ( X&& ...x ) const {
    return T<X...>(std::forward<X>(x)...);
  }
};

/// Forwarder -- A function type-erasure.
///
/// Base case: Function pointers and objects.
/// Lifts functions pointers to objects and acts as a type erasure between
/// function pointers and objects.
template<class F>
struct Forwarder : public F {
  F f;
  constexpr Forwarder(F f) : f(std::move(f)) { }
  
  template<class...X>
  constexpr auto operator() (X&&...x) const
    -> std::result_of_t<F(X...)>
  {
    return f(std::forward<X>(x)...);
  }
};

/// Forwarder: Function reference specialization.
template<class R, class...X>
struct Forwarder<R(X...)> {
  // Function type must be converted to a pointer.
  using type = R(*)(X...);

  // Note: gcc 4.9 will not consider `type f` a constexpr.
  R(*f)(X...);
  constexpr Forwarder(type f) : f(f) { }

  constexpr R operator() (X&&...x) const {
    return f(std::forward<X>(x)...);
  }
};

/// Forwarder: Function pointer specialization.
template<class R, class...X>
struct Forwarder<R(*)(X...)> : Forwarder<R(X...)> {
  using base = Forwarder<R(X...)>;
  using base::base;
  using base::operator();
};

/// Non-std mem_fn (Like std::mem_fn, but can be constexpr).
template<typename F>
struct MemFn {
  F f;

  // True if `f` points to a member object, not a function.
  static constexpr bool not_f = std::is_member_object_pointer<F>::value;

  constexpr MemFn(F f) : f(f) { }

  template<class O, class...X>
  constexpr auto operator()(O&& o, X&&...x) const
    -> decltype((std::declval<O>().*f)(std::declval<X>()...))
  {
    return (std::forward<O>(o).*f)(std::forward<X>(x)...);
  }

  template<class O, class...X>
  constexpr auto operator() (O&& o, X&&...x) const
    -> decltype((std::declval<O>()->*f)(std::declval<X>()...))
  {
    return (std::forward<O>(o)->*f)(std::forward<X>(x)...);
  }

  template<class O>
  constexpr auto operator() (O&& o) const
    -> enable_if_t<not_f, decltype(std::declval<O>().*f)>
  {
    return std::forward<O>(o).*f;
  }

  template<class O>
  constexpr auto operator() (O&& o) const
    -> enable_if_t<not_f, decltype(std::declval<O>()->*f)>
  {
    return std::forward<O>(o)->*f;
  }
};

/// MemFn constructor.
template<class F>
constexpr MemFn<std::decay_t<F>> mem_fn(F&& f) {
  return {std::forward<F>(f)};
}

/// to_functor: Ensures function, f, is an object.
template<class F>
constexpr F to_functor(F&& f) { return std::forward<F>(f); }

/// Function pointer overload: Lifts `f` to a function object.
template<class R, class...X>
constexpr Forwarder<R(X...)> to_functor(R(*f)(X...)) { return f; }

/// Member function overload: Lifts `f` using MemFn.
template<class T, class O>
constexpr auto to_functor(T O::*f) { return mem_fn(f); }

/// Makes a function-object type out of `F`.
template<class F>
using ToFunctor = decltype(to_functor(std::declval<F>()));

namespace tpl {
  /// apply(f, {x...}) = f(x...)
  constexpr struct apply_f {
    template<size_t i, class Tuple>
    using Elem = decltype(std::get<i>(std::declval<Tuple>()));

    template<class F, class Tuple, class I, I...N>
    constexpr auto operator() (const F& f, Tuple&& t,
                               std::integer_sequence<I, N...>) const
      -> std::result_of_t<F(Elem<N,Tuple>...)>
    {
      return f(std::get<N>(std::forward<Tuple>(t))...);
    }

    template<class F, class Tuple>
    constexpr auto operator() (const F& f, Tuple&& t) const
      -> decltype((*this)(f, std::forward<Tuple>(t), iseq::make(t)))
    {
      return (*this)(f, std::forward<Tuple>(t), iseq::make(t));
    }

    // Since apply_f is used to define generic partial application, it must
    // define the partial application of itself with a single function.
    template<class F>
    struct apply_1_f {
      F f;
      constexpr apply_1_f(F f) : f(std::move(f)) { }

      template<class Tuple>
      constexpr auto operator() (Tuple&& t) const
        -> std::result_of_t<apply_f(F, Tuple)>
      {
        return apply_f{}(f, std::forward<Tuple>(t));
      }
    };

    template<class F>
    constexpr apply_1_f<F> operator() (F f) const {
      return {std::move(f)};
    }
  } apply{};
} // namespace tpl

/// Partial Application
template<class F, class...X>
struct Part {
  ToFunctor<F> f;

  std::tuple<X...> t;

  constexpr Part(F f, X...x) : f(std::move(f))
                             , t(std::forward<X>(x)...)
  {
  }

  template<class...Y, class Tuple = std::tuple<Y...>>
  static constexpr Tuple args(Y&&...y) {
    return Tuple(std::forward<Y>(y)...);
  }

  template<class...Y>
  constexpr std::result_of_t<const F&(X..., Y...)> operator() (Y&&...y) const & {
    return tpl::apply(f, std::tuple_cat(t, args(std::forward<Y>(y)...)));
  }

  template<class...Y>
  constexpr std::result_of_t<F&&(X..., Y...)> operator() (Y&&...y) && {
    return tpl::apply(std::move(f),
                      std::tuple_cat(std::move(t), args(std::forward<Y>(y)...)));
  }
};

/// Closure: partial application by copying the parameters.
/// Given f(x,y,z):
///   closure(f,x) = g(y,z) = f(x,y,z)
///   closure(f,x,y) = g(z) = f(x,y,z)
constexpr auto closure = MakeT<Part>{};

/// Like closure, but forwards its arguments.
constexpr auto part = ForwardT<Part>{};

/// A function that takes two or more arguments. If given only one argument, it
/// will return a partial application.
template<class F>
struct Multary : ToFunctor<F> {
  using Fn = ToFunctor<F>;
  using Fn::operator();

  constexpr Multary(F f) : Fn(std::move(f)) { }

  template<class X>
  using Partial = decltype(closure(std::declval<Multary>(), std::declval<X>()));

  template<class X>
  constexpr Partial<X> operator() (X x) const & {
    return closure(*this, std::move(x));
  }

  template<class X>
  constexpr Partial<X> operator() (X x) const && {
    return closure(std::move(*this), std::move(x));
  }
};

/// A function that takes two or more arguments. If given only one argument, it
/// will return a partial application.
constexpr auto multary = MakeT<Multary>{};

/// A function that takes three or more arguments. If given only one or two
/// arguments, it will return a partial application.
template<class F>
struct Multary2 : Multary<F> {
  using Multary<F>::operator();

  constexpr Multary2(F f) : Multary<F>(std::move(f)) { }

  template<class X, class Y>
  constexpr Part<F, X, Y> operator() (X x, Y y) const & {
    return closure(*this, std::move(x), std::move(y));
  }

  template<class X, class Y>
  constexpr Part<F, X, Y> operator() (X x, Y y) const && {
    return closure(std::move(*this), std::move(x), std::move(y));
  }
};

/// A function that takes two or more arguments. If given only one argument, it
/// will return a partial application.
constexpr auto multary2 = MakeT<Multary2>{};

} // namspace fu
