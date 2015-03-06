
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

/// forwarder_f -- A function type-erasure.
///
/// Base case: Function pointers and objects.
/// Lifts functions pointers to objects and acts as a type erasure between
/// function pointers and objects.
template<class F>
struct forwarder_f : public F {
  F f;
  constexpr forwarder_f(F f) : f(std::move(f)) { }
  
  template<class...X>
  constexpr decltype(auto) operator() (X&&...x) const {
    return f(std::forward<X>(x)...);
  }
};

/// forwarder_f: Function reference specialization.
template<class R, class...X>
struct forwarder_f<R(X...)> {
  // Function type must be converted to a pointer.
  using type = R(*)(X...);

  // Note: gcc 4.9 will not consider `type f` a constexpr.
  R(*f)(X...);
  constexpr forwarder_f(type f) : f(f) { }

  constexpr R operator() (X&&...x) const {
    return f(std::forward<X>(x)...);
  }
};

/// forwarder_f: Function pointer specialization.
template<class R, class...X>
struct forwarder_f<R(*)(X...)> : forwarder_f<R(X...)> {
  using base = forwarder_f<R(X...)>;
  using base::base;
  using base::operator();
};

/// Non-std mem_fn (Like std::mem_fn, but can be constexpr).
template<typename F>
struct MemFn {
  F f;

  constexpr MemFn(F f) : f(f) { }

  template<class O, class...X>
  constexpr decltype(auto) operator() (O&& o, X&&...x) const {
    return invoke_member(f, std::forward<O>(o), std::forward<X>(x)...);
  }
};

template<class R, class O>
struct MemFn<R O::*> {
  using F = R O::*;
  F f;

  constexpr MemFn(F f) : f(f) { }

  constexpr decltype(auto) operator() (O& o) const {
    return invoke_member(f, o);
  }

  constexpr decltype(auto) operator() (O&& o) const {
    return invoke_member(f, std::move(o));
  }

  constexpr decltype(auto) operator() (const O& o) const {
    return invoke_member(f, o);
  }

  constexpr decltype(auto) operator() (const O&& o) const {
    return invoke_member(f, std::move(o));
  }

  constexpr decltype(auto) operator() (O* o) const {
    return invoke_member(f, o);
  }

  constexpr decltype(auto) operator() (const O* o) const {
    return invoke_member(f, o);

  }
};

/// Basic member function object given a member function, F, and an object
/// type, O.
template<class F, class O>
struct basic_mem_fn_f {
  F f;

  constexpr basic_mem_fn_f(F f) : f(f) { }

  template<class...X>
  constexpr decltype(auto) operator()(O&& o, X&&...x) const {
    return invoke_member(f, std::forward<O>(o), std::forward<X>(x)...);
  }
};


template<class R, class O, class...X>
struct MemFn<R (O::*)(X...)> : basic_mem_fn_f<R (O::*)(X...), O&> {
  using base = basic_mem_fn_f<R (O::*)(X...), O&>;
  using base::base;
  using base::operator();
};

template<class R, class O, class...X>
struct MemFn<R (O::*)(X...) &> : basic_mem_fn_f<R (O::*)(X...) &, O&> {
  using base = basic_mem_fn_f<R (O::*)(X...) &, O&>;
  using base::base;
  using base::operator();
};

template<class R, class O, class...X>
struct MemFn<R (O::*)(X...) &&> : basic_mem_fn_f<R (O::*)(X...) &&, O> {
  using base = basic_mem_fn_f<R (O::*)(X...) &&, O>;
  using base::base;
  using base::operator();
};

template<class R, class O, class...X>
struct MemFn<R (O::*)(X...) const>
    : basic_mem_fn_f<R (O::*)(X...) const, const O&>
{
  using base = basic_mem_fn_f<R (O::*)(X...) const, const O&>;
  using base::base;
  using base::operator();
};

template<class R, class O, class...X>
struct MemFn<R (O::*)(X...) const&>
    : basic_mem_fn_f<R (O::*)(X...) const&, const O&>
{
  using base = basic_mem_fn_f<R (O::*)(X...) const&, const O&>;
  using base::base;
  using base::operator();
};

template<class R, class O, class...X>
struct MemFn<R (O::*)(X...) const&&>
    : basic_mem_fn_f<R (O::*)(X...) const&&, const O>
{
  using base = basic_mem_fn_f<R (O::*)(X...) const&&, const O>;
  using base::base;
  using base::operator();
};

/// MemFn constructor.
template<class F>
constexpr MemFn<F> mem_fn(F f) {
  return {f};
}

/// forwarder: Ensures function, f, is an object.
template<class F>
constexpr F forwarder(F&& f) { return std::forward<F>(f); }

/// Function pointer overload: Lifts `f` to a function object.
template<class R, class...X>
constexpr forwarder_f<R(X...)> forwarder(R(*f)(X...)) { return f; }

/// Member function overload: Lifts `f` using MemFn.
template<class T, class O>
constexpr auto forwarder(T O::*f) { return mem_fn(f); }

/// Makes a function-object type out of `F`.
template<class F>
using ToFunctor = decltype(forwarder(std::declval<F>()));

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
    return tpl::apply(f, t, args(std::forward<Y>(y)...));
  }

  template<class...Y>
  constexpr RESULT(const F&&) operator() (Y&&...y) && {
    return tpl::apply(std::move(f), std::move(t), args(std::forward<Y>(y)...));
  }

#ifdef __clang__
  template<class...Y>
  constexpr RESULT(F&) operator() (Y&&...y) & {
    return tpl::apply(f, t, args(std::forward<Y>(y)...));
  }

  template<class...Y>
  constexpr RESULT(const F&&) operator() (Y&&...y) const && {
    return tpl::apply(std::move(f), std::move(t), args(std::forward<Y>(y)...));
  }
#endif

#undef RESULT
};

/// Reversed-Partial Application
template<class F, class...X>
struct rpart_f {
  F f;

  std::tuple<X...> t;

  constexpr rpart_f(F f, X...x) : f(std::move(f))
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
# define RESULT(F) std::result_of_t<F(Y..., X...)>
#endif

  template<class...Y>
  constexpr RESULT(const F&) operator() (Y&&...y) const & {
    return tpl::apply(f, args(std::forward<Y>(y)...), t);
  }

  template<class...Y>
  constexpr RESULT(const F&&) operator() (Y&&...y) && {
    return tpl::apply(std::move(f), args(std::forward<Y>(y)...), std::move(t));
  }

#ifdef __clang__
  template<class...Y>
  constexpr RESULT(F&) operator() (Y&&...y) & {
    return tpl::apply(f, args(std::forward<Y>(y)...), t);
  }

  template<class...Y>
  constexpr RESULT(const F&&) operator() (Y&&...y) const && {
    return tpl::apply(std::move(f), args(std::forward<Y>(y)...), std::move(t));
  }
#endif

#undef RESULT
};


/// Closure: partial application by copying the parameters.
/// Given f(x,y,z):
///   closure(f,x) = g(y,z) = f(x,y,z)
///   closure(f,x,y) = g(z) = f(x,y,z)
constexpr auto closure = MakeT<Part>{};

/// Like closure, but forwards its arguments.
constexpr auto part = ForwardT<Part>{};

/// Closure: reverse partial application by copying the parameters.
/// Given f(x,y,z):
///   closure(f,z) = g(x,y) = f(x,y,z)
///   closure(f,y,z) = g(x) = f(x,y,z)
constexpr auto rclosure = MakeT<rpart_f>{};

/// Like closure, but forwards its arguments.
constexpr auto rpart = ForwardT<rpart_f>{};

/// A function that takes `n` or more arguments. If given only one argument, it
/// will return a partial application.
template<size_t n, class _F>
struct multary_n_f : ToFunctor<_F> {
  using F = ToFunctor<_F>;

  constexpr multary_n_f(F f) : F(std::move(f)) { }

  // The result of applying this m arguments where m <= n.
  template<class...X>
  using Partial = multary_n_f<n - sizeof...(X), Part<F, X...>>;

  /// Too few arguments: Return another multary function.
  template<class...X, class = enable_if_t<(sizeof...(X) < n)>>
  constexpr Partial<X...> operator() (X...x) const & {
    return Partial<X...>(closure(F(*this), std::move(x)...));
  }

  /// Exactly n arguments: Partially apply.
  template<class...X, class = enable_if_t<(sizeof...(X) == n)>>
  constexpr Part<F, X...> operator() (X...x) const & {
    return closure(F(*this), std::move(x)...);
  }

  /// More than n arguments: invoke.
  template<class...X, class = enable_if_t<(sizeof...(X) > n)>>
  constexpr decltype(auto) operator() (X&&...x) const &
  {
    return static_cast<const F&>(*this)(std::forward<X>(x)...);
  }
};

template<class F>
struct multary_n_f<0, F> : ToFunctor<F> {
  using Fn = ToFunctor<F>;
  using Fn::operator();
};

/// A function that takes two or more arguments. If given only one argument, it
/// will return a partial application.
constexpr struct multary_f {
  // TODO: Make the n an std::integer_constant so that
  //       basic_multary = MakeT<multary_n_f>{};
  //       multary = part(basic_multary, Int<0>{})
  template<class F>
  constexpr multary_n_f<1, F> operator() (F f) const {
    return multary_n_f<1, F>(std::move(f));
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
  constexpr auto operator() (F f) const -> multary_n_f<n, F>
  {
    return {std::move(f)};
  }

  template<class F>
  constexpr auto operator() (std::reference_wrapper<F> f) const
    -> multary_n_f<n, F&>
  {
    return {std::move(f)};
  }
};

template<size_t n>
constexpr auto multary_n = make_multary_n_f<n>{};
#else
template<size_t n, class F>
constexpr multary_n_f<n, F> multary_n(F f) {
  return multary_n_f<n, F>(std::move(f));
}
#endif  // __clang__

} // namspace fu
