
/// General function-object utilities.

#pragma once

#include <utility>
#include <functional>

#include <fu/basic.h>

namespace fu {

/// Rank: Uses inheritance to communicate precedence in overloading.
///
/// See: http://flamingdangerzone.com/cxx11/2013/03/11/overload-ranking.html
template<size_t N>
struct Rank : Rank<N-1> {
};

/// Rank base case: All Ranks inherit from Rank<0>.
template<>
struct Rank<0> {
};

struct lassoc_f {
  template<class F, class X, class Y>
  constexpr decltype(auto) operator() (F&& f, X&& x, Y&& y) const {
    return invoke(std::forward<F>(f), std::forward<X>(x), std::forward<Y>(y));
  }

  template<class F, class X, class Y, class...Z,
           class = enable_if_t<(sizeof...(Z) > 0)>>
  constexpr decltype(auto) operator() (const F& f, X&& x, Y&& y, Z&&...z) const {
    return (*this)(f,
                   invoke(f, std::forward<X>(x), std::forward<Y>(y)),
                   std::forward<Z>(z)...);
  }
};

/// Right-associative application.
struct rassoc_f {
  template<class F, class X, class Y>
  constexpr decltype(auto) operator() (F&& f, X&& x, Y&& y) const {
    return invoke(std::forward<F>(f), std::forward<X>(x), std::forward<Y>(y));
  }

  template<class F, class X, class...Y
          ,class = std::enable_if_t<(sizeof...(Y) > 1)>>
  constexpr decltype(auto) operator() (const F& f, X&& x, Y&&...y) const {
    return invoke(f, std::forward<X>(x),
                  (*this)(f, std::forward<Y>(y)...));
  }
};

/// Left-associative application.
///
/// Ex: lassoc(+)(1,2,3) = (1+2) + 3
constexpr auto lassoc = multary(lassoc_f{});

struct transitive_f {
  /// trans(b,j,x,y) = b(x,y)
  template<class Binary, class Join, class X, class Y>
  constexpr auto operator() (Binary&& b, const Join&, X&& x, Y&& y) const {
    return fu::invoke(std::forward<Binary>(b),
                      std::forward<X>(x), std::forward<Y>(y));
  }

  /// trans(b,j,x,y,z...) = j(b(x,y), b(y,z...))
  template<class Binary, class Join, class X, class Y, class...Z,
           class = enable_if_t<(sizeof...(Z) > 0)>>
  constexpr auto operator() (const Binary& b, const Join& j,
                             X&& x, const Y& y, Z&&...z) const
  {
    return fu::invoke(j,
                      fu::invoke(b, std::forward<X>(x), y),
                      (*this)(b, j, y, std::forward<Z>(z)...));
  }
};

/// transitive(b,j) -- produces a function, t, such that
///   t(x,y)    <=>  b(x,y)
///   t(x,y,z)  <=>  j(b(x,y), b(y,z))
///
/// Ex: t = transitive(<, &&)
///   t(x,y)    ==  x < y
///   t(x,y,z)  ==  x < y && y < z
constexpr auto transitive = multary_n<2>(transitive_f{});

/// A function object that lifts C++ overloading rules to a type class.
template<class F, class G>
struct Overloaded : public ToFunctor<F>, public ToFunctor<G> {
  constexpr Overloaded(F f, G g) : ToFunctor<F>(std::move(f)),
                                   ToFunctor<G>(std::move(g))
  { }

  using ToFunctor<F>::operator();
  using ToFunctor<G>::operator();
};

/// Constructs and overloaded function.
/// Ex:
///   void f(int);
///   void g(float);
///   auto o = overload(f,g);
///   o(1);     // calls f(int)
///   o(0.0f);  // calls g(float)
constexpr auto overload = lassoc(MakeT<Overloaded>{});

/// Like Overloaded, but uses Rank to decide whether to call F (preferred) or G.
template<class F, class G>
struct RankOverloaded {
  ToFunctor<F> f;
  ToFunctor<G> g;

  constexpr RankOverloaded(F f, G g) : f(std::move(f))
                                     , g(std::move(g))
  { }

  template<class...X>
  constexpr auto call(Rank<1>, X&&...x) const
    -> decltype(f(std::declval<X>()...))
  {
    return f(std::forward<X>(x)...);
  }

  template<class...X>
  constexpr auto call(Rank<0>, X&&...x) const
    -> decltype(g(std::declval<X>()...))
  {
    return  g(std::forward<X>(x)...);
  }
  
  template<class...X>
  constexpr auto operator() (X&&...x) const
  // NOTE: GCC fails to compile without the "this->".
    -> decltype(this->call(Rank<1>{}, std::declval<X>()...))
  {
    return call(Rank<1>{}, std::forward<X>(x)...);
  }
};

/// Like overload(), but uses Rank to dispatch which function to call,
/// preferring the first over the last.
constexpr auto ranked_overload = lassoc(MakeT<RankOverloaded>{});

struct compose_f {
  /// Applies the inner function; returns a tuple so that it can be
  /// concatenated with the arguments for the outer function.
  template<class G, class Tuple>
  static constexpr decltype(auto) app1(G&& g, Tuple&& t) {
    return tpl::forward_tuple(tpl::apply(std::forward<G>(g),
                                         std::forward<Tuple>(t)));
  }

  template<class F, class G, class TupleA, class TupleB>
  constexpr auto operator() (F&& f, G&& g, TupleA&& a, TupleB&& b) const
    -> decltype(auto)
  {
    return tpl::apply(std::forward<F>(f),
                      app1(std::forward<G>(g), std::forward<TupleA>(a)),
                      std::forward<TupleB>(b));
  }
};

/// Unary Composition
///
/// Lemma:
///   (f . g)(x) = f(g(x))
///   (f . g)(x,y,z) = f(g(x), y, z)
struct ucompose_f {
  template<class F, class G, class X, class...Y>
  constexpr decltype(auto) operator() (F&& f, G&& g, X &&x, Y&&...y) const {
    return fu::invoke(std::forward<F>(f),
                      fu::invoke(std::forward<G>(g), std::forward<X>(x)),
                      std::forward<Y>(y)...);
  }
};

/// Creates a unary composition: ucompose(f,g)(x) = f(g(x))
constexpr auto ucompose = lassoc(multary_n<2>(ucompose_f{}));

struct fix_f {
  template<class F>
  constexpr auto rec(const F& f) const {
    return part(fix_f{}, f);
  }

  template<class F, class...X>
  
#ifndef __clang__
  constexpr
#endif
  decltype(auto) operator() (const F& f, X&&...x) const
  {
    return f(rec(f), std::forward<X>(x)...);
  }
};

/// Fixed-point combinator: fix(f,x) <=> f(fix(f), x)
/// http://en.wikipedia.org/wiki/Fixed-point_combinator
///
/// Allows one to define a recursive function without referring to itself.
/// `f` must accept a function as its first argument and call that function
/// in order to recurse. `f` must not use `auto` as the return type to
/// prevent "use of f before deduction of auto" errors. Due to a bug in
/// clang (http://llvm.org/bugs/show_bug.cgi?id=20090), `f` may not be
/// marked `constexpr`.
///
/// Ex:
///   constexpr int pow2 = fix([](auto rec, int x) -> int {
///     return x ? 2 * rec(x-1) : 1;
///   });
constexpr auto fix = multary(fix_f{});

/// Creates a generic composition.
///
/// compose(f,g)({x...}, {y...}) = f(g(x...), y...)
constexpr auto compose = multary_n<2>(compose_f{});
// FIXME: Should be `lassoc`, but the current definition won't work.

template<size_t n>
struct compose_n_f {
  template<std::size_t...i, std::size_t...j, class F, class G, class Tuple>
  static constexpr decltype(auto) do_invoke(std::index_sequence<i...>,
                                            std::index_sequence<j...>,
                                            F&& f, G&& g, Tuple&& t) {
    return invoke(std::forward<F>(f),
                  invoke(std::forward<G>(g),
                         std::get<i>(std::forward<Tuple>(t))...),
                  std::get<j>(std::forward<Tuple>(t))...);
  }


  template<class F, class G, class Tuple>
  static constexpr decltype(auto) split(F&& f, G&& g, Tuple&& t) {
    using is = decltype(iseq::make(t));
    return do_invoke(iseq::take<n>(is{}), iseq::drop<n>(is{}),
                     std::forward<F>(f),
                     std::forward<G>(g),
                     std::forward<Tuple>(t));
  }

  template<class F, class G, class...X>
  constexpr decltype(auto) operator() (F&& f, G&& g, X&&...x) const {
    return split(std::forward<F>(f), std::forward<G>(g),
                 tpl::forward_tuple(std::forward<X>(x)...));
  }
};

template<size_t n, class F, class G>
constexpr auto compose_n(F f, G g) {
  return multary_n<n>(closure(compose_n_f<n>{}, std::move(f), std::move(g)));
}

struct proj_f {
  template<class F, class ProjF, class...X,
           class = enable_if_t<(sizeof...(X) > 0)>>
  constexpr decltype(auto) operator() (F&& f, const ProjF& pf, X&&...x) const &
  {
    return invoke(std::forward<F>(f),
                  invoke(pf, std::forward<X>(x))...);
  }
};

struct lproj_f {
  template<class F, class ProjF, class X, class Y>
  constexpr decltype(auto) operator() (F&& f, ProjF&& pf, X&& x, Y&& y) const {
    return invoke(std::forward<F>(f),
                  invoke(std::forward<ProjF>(pf),
                         std::forward<X>(x)),
                  std::forward<Y>(y));
  }
};

/// proj(f,pf) -- constructs a projection, p, such that
///   p(x,y) <=> f(pf(x), pf(y))
///   p(x)   <=> ucompose(f, pf)(x)
constexpr auto proj = multary_n<2>(proj_f{});

/// lproj(f, pf, x, y) <=> f(pf(x), y)
constexpr auto lproj = multary_n<2>(lproj_f{});

struct _less_helper {
  template<class X, class Y>
  constexpr bool operator() (const X& x, const Y& y) const {
    return x < y;
  }
};

/// proj_less(pf, x, y) <=> pf(x) < pf(y)
///
/// Useful for use with <algorithm> sorting/comparing functions.
constexpr auto proj_less = proj(_less_helper{});

struct join_f {
  template<class F, class Left, class Right, class X, class Y>
  constexpr decltype(auto) operator() (F&& f, Left&& l, Right&& r,
                                       X&& x, Y&& y) const
  {
    return invoke(std::forward<F>(f),
                  invoke(std::forward<Left>(l), std::forward<X>(x)),
                  invoke(std::forward<Right>(r), std::forward<Y>(y)));
  }
};

/// join(f,l,r) -- constructs a projection, j, such that
///   j(x,y) <=> f(l(x), r(y))
// TODO: multary3?
constexpr auto join = multary_n<3>(join_f{});

struct split_f {
  template<class F, class Left, class Right, class X>
  constexpr decltype(auto) operator() (F&& f, Left&& l, Right&& r, X& x) const
  {
    return invoke(std::forward<F>(f),
                  invoke(std::forward<Left>(l), x),
                  invoke(std::forward<Right>(r), x));
  }
};

/// split(f,l,r)(x) <=> f(l(x), r(x))
constexpr auto split = multary_n<3>(split_f{});

struct flip_f {
  template<class I, I...i, class F, class...X>
  static constexpr decltype(auto) reverse(std::integer_sequence<I,i...>,
                                          F&& f,
                                          std::tuple<X...> args)
  {
    return fu::invoke(std::forward<F>(f),
                      std::get<sizeof...(X) - i - 1>(std::move(args))...);
  }

  template<class F, class...X>
  constexpr decltype(auto) operator() (F&& f, X&&...x) const {
    return reverse(std::index_sequence_for<X...>{},
                   std::forward<F>(f),
                   tpl::forward_tuple(std::forward<X>(x)...));
  }
};

constexpr auto flip = multary_n<2>(flip_f{});

/// pipe(x,f,g) <=> g(f(x))
/// pipe(x)     <=> x
constexpr auto pipe = overload(identity, lassoc(flip(invoke)));
// Informal proof:
// lassoc(flip(invoke), x, f, g)         =
//   flip(invoke)(flip(invoke)(x, f), g) =  | definition of lassoc
//   invoke(g, invoke(f,x)) = g(f(x))       | definition of flip

/// rproj(f, pf, x, y) <=> f(x, pf(y))
constexpr auto rproj = multary_n<2>(ucompose(compose_n<2>(flip, lproj), flip));
// Informal proof:
// compose(compose<2>(flip, lproj), flip))(f)(pf)(x, y) =
//   compose<2>(flip, lproj)(flip(f), pf)(x, y)  =  | definition of compose
//   flip(lproj(flip(f) pf))(x, y)               =  | definition of compose<2>
//   lproj(flip(f), pf, y, x)                    =  | definition of flip
//   flip(f)(pf(y) x) = f(x, pf(y))                 | definition of lproj

/// Right-associative application.
/// rassoc(f, x, y, z) <=> f(x, f(y,z))
///
/// Ex: rassoc(+)(1,2,3) = 1 + (2+3)
constexpr auto rassoc = multary(ucompose(flip, lassoc, flip));
// Informal proof:
// compose(flip, lassoc, flip)(f)(x,y,z) =
//  flip(lassoc(flip(f)))(x,y,z) =  | definition of compose
//  lassoc(flip(f), z, y, x)     =  | definition if flip and invoke
//  flip(f)(flip(f,z,y), x)      =  | definition of lassoc
//  f(x, f(y,z))
//
// The extra `invoke` is required to make sure `flip . lassoc . flip` doesn't
// get applied more than one argument.

template<template<class...>class Enabler, class F>
struct Enabled_f {
  F f;

  constexpr Enabled_f(F f) : f(std::move(f)) { }

  // FIXME: Should be able to take more than one argument, but compiler
  // complains.
  template<class X, class = enable_if_t<Enabler<X>::value>>
  constexpr decltype(auto) operator() (X&& x) const {
    return invoke(f, std::forward<X>(x));
  }
};

template<template<class...>class Enabler, class F>
constexpr Enabled_f<Enabler, F> enable_if_f(F f) {
  return {std::move(f)};
}

template<class X>
struct Constant {
  X x;

  constexpr Constant(X x) : x(std::move(x)) { }

  const X& operator() () const& { return x; }
  X&       operator() () &      { return x; }
  X        operator() () &&     { return std::move(x); }
};

constexpr auto constant = MakeT<Constant>{};

} // namespace fu
