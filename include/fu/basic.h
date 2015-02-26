
#pragma once

#include <utility>
#include <functional>

#include <fu/invoke.h>
#include <fu/iseq.h>
#include <fu/make/make.h>
#include <fu/tuple/basic.h>

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
  constexpr decltype(auto) operator() (X&&...x) const {
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

/// Partial Application
template<class F, class...X>
struct Part {
  F f;

  std::tuple<X...> t;

  constexpr Part(F f, X...x) : f(std::move(f))
                             , t(std::forward<X>(x)...)
  {
  }

  template<class...Y, class Tuple = std::tuple<Y...>>
  static constexpr Tuple args(Y&&...y) {
    return Tuple(std::forward<Y>(y)...);
  }

  // NOTE: due to gcc bug, decltype(auto) may not be used to define operator()
  // because the wrong overloads will be chosen.
  // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=64562
#ifdef __clang__
# define RESULT(F) decltype(auto)
#else
# define RESULT(F) std::result_of_t<F(X..., Y...)>
#endif

  template<class...Y>
  constexpr RESULT(const F&) operator() (Y&&...y) const & {
    return tpl::apply(f, std::tuple_cat(t, args(std::forward<Y>(y)...)));
  }

  template<class...Y>
  constexpr RESULT(const F&&) operator() (Y&&...y) && {
    return tpl::apply(std::move(f),
                      std::tuple_cat(std::move(t), args(std::forward<Y>(y)...)));
  }

#ifdef __clang__
  template<class...Y>
  constexpr RESULT(F&) operator() (Y&&...y) & {
    return tpl::apply(f, std::tuple_cat(t, args(std::forward<Y>(y)...)));
  }

  template<class...Y>
  constexpr RESULT(const F&&) operator() (Y&&...y) const && {
    return tpl::apply(std::move(f),
                      std::tuple_cat(std::move(t), args(std::forward<Y>(y)...)));
  }
#endif
};


/// Closure: partial application by copying the parameters.
/// Given f(x,y,z):
///   closure(f,x) = g(y,z) = f(x,y,z)
///   closure(f,x,y) = g(z) = f(x,y,z)
constexpr auto closure = MakeT<Part>{};

/// Like closure, but forwards its arguments.
constexpr auto part = ForwardT<Part>{};

/// A function that takes `n` or more arguments. If given only one argument, it
/// will return a partial application.
template<size_t n>
struct multary_n_f {
  // The result of applying this m arguments where m <= n.
  template<class F, class...X>
  using Partial = Part<multary_n_f<n - sizeof...(X)>, Part<F,X...>>;

  /// Too few arguments: Return another multary function.
  template<class F, class...X, class = enable_if_t<(sizeof...(X) < n)>>
  constexpr auto operator() (F f, X...x) const {
    return part(multary_n_f<n - sizeof...(X)>{},
                closure(std::move(f), std::move(x)...));
  }

  /// Exactly n arguments: Partially apply.
  template<class F, class...X, class = enable_if_t<(sizeof...(X) == n)>>
  constexpr auto operator() (F&& f, X...x) const {
    return closure(std::forward<F>(f), std::move(x)...);
  }

  /// More than n arguments: invoke.
  template<class F, class...X, class = enable_if_t<(sizeof...(X) > n)>>
  constexpr decltype(auto) operator() (F&& f, X&&...x) const
  {
    return fu::invoke(std::forward<F>(f), std::forward<X>(x)...);
  }
};

template<>
struct multary_n_f<0> : invoke_f {
  using invoke_f::operator();
};

/// A function that takes two or more arguments. If given only one argument, it
/// will return a partial application.
constexpr struct multary_f {
  // TODO: Make the n an std::integer_constant so that
  //       basic_multary = MakeT<multary_n_f>{};
  //       multary = part(basic_multary, Int<0>{})
  template<class F>
  constexpr auto operator() (F f) const {
    return closure(multary_n_f<1>{}, std::move(f));
  }
} multary{};

/// A function that takes two or more arguments. If given only one argument, it
/// will return a partial application.
#ifdef __clang__
// For clang, experimentally define multary_n<n> as a template variable. GCC
// earlier than version 5 still cannot handle them.
template<size_t n>
struct make_multary_n_f {
  template<class F>
  constexpr auto operator() (F f) const {
    return closure(multary_n_f<n>{}, std::move(f));
  }

  template<class F>
  constexpr auto operator() (std::reference_wrapper<F> f) const {
    return part(multary_n_f<n>{}, f.get());
  }
};

template<size_t n>
constexpr auto multary_n = make_multary_n_f<n>{};
#else
template<size_t n, class F>
constexpr auto multary_n(F f) {
  return closure(multary_n_f<n>{}, std::move(f));
}
#endif  // __clang__

} // namspace fu
