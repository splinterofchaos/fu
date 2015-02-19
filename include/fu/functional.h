
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

constexpr struct pipe_f {
  template<class X>
  constexpr X operator() (X&& x) const {
    return std::forward<X>(x);
  }

  template<class X, class F, class...G>
  constexpr auto operator() (X&& x, F&& f, G&&...g) const 
    -> std::result_of_t<pipe_f(std::result_of_t<F(X)>, G...)>
  {
    return (*this)(invoke(std::forward<F>(f), std::forward<X>(x)),
                   std::forward<G>(g)...);
  }
} pipe{};

struct lassoc_f {
  template<class F, class X, class Y>
  constexpr auto operator() (F&& f, X&& x, Y&& y) const
    -> std::result_of_t<F(X,Y)>
  {
    return invoke(std::forward<F>(f), std::forward<X>(x), std::forward<Y>(y));
  }

  template<class F, class X, class Y, class...Z,
           class = enable_if_t<(sizeof...(Z) > 0)>>
  constexpr auto operator() (const F& f, X&& x, Y&& y, Z&&...z) const {
    return (*this)(f,
                   invoke(f, std::forward<X>(x), std::forward<Y>(y)),
                   std::forward<Z>(z)...);
  }
};

/// Right-associative application.
struct rassoc_f {
  template<class F, class X, class Y>
  constexpr auto operator() (F&& f, X&& x, Y&& y) const
    -> std::result_of_t<F(X,Y)>
  {
    return invoke(std::forward<F>(f), std::forward<X>(x), std::forward<Y>(y));
  }

  template<class F, class X, class...Y
          ,class = std::enable_if_t<(sizeof...(Y) > 1)>>
  constexpr auto operator() (const F& f, X&& x, Y&&...y) const
    -> std::result_of_t<F(X, std::result_of_t<rassoc_f(F,Y...)>)>
  {
    return invoke(f, std::forward<X>(x),
                  (*this)(f, std::forward<Y>(y)...));
  }
};

/// Left-associative application.
///
/// Ex: lassoc(+)(1,2,3) = (1+2) + 3
constexpr auto lassoc = multary(lassoc_f{});

/// Right-associative application.
///
/// Ex: rassoc(+)(1,2,3) = 1 + (2+3)
constexpr auto rassoc = multary(rassoc_f{});

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

template<class F, class G>
struct Composition {
  F f;
  G g;

  constexpr Composition(F f, G g) : f(std::move(f))
                                  , g(std::move(g))
  {
  }

  template<class Tuple>
  constexpr decltype(auto) app1(Tuple t) const & {
    return tpl::forward_tuple(tpl::apply(g, std::forward<Tuple>(t)));
  }

  template<class Tuple>
  constexpr decltype(auto) app1(Tuple t) const && {
    return tpl::forward_tuple(tpl::apply(std::move(g), std::forward<Tuple>(t)));
  }

  template<class TupleA, class TupleB>
  constexpr decltype(auto) operator() (TupleA&& a, TupleB&& b) const & {
    return tpl::apply(f,
                      std::tuple_cat(app1(std::forward<TupleB>(b)),
                                     std::forward<TupleB>(b)));
  }

  template<class TupleA, class TupleB>
  constexpr decltype(auto) operator() (TupleA&& a, TupleB&& b) const && {
    return tpl::apply(std::move(f),
                      std::tuple_cat(app1(std::forward<TupleB>(b)),
                                     std::forward<TupleB>(b)));
  }
};

/// Unary Composition
///
/// Lemma:
///   (f . g)(x) = f(g(x))
///   (f . g)(x,y,z) = f(g(x), y, z)
template<class F, class G>
struct UComposition : Composition<F,G> {
  using base = Composition<F,G>;

  constexpr UComposition(F f, G g) : base(std::move(f), std::move(g))
  {
  }

  template<class X, class...Y>
  constexpr decltype(auto) operator() (X &&x, Y&&...y) const & {
    return (*this)(tpl::forward_tuple(std::forward<X>(x)),
                   tpl::forward_tuple(std::forward<Y>(y)...));
  }

  template<class X, class...Y>
  constexpr decltype(auto) operator() (X &&x, Y&&...y) const && {
    return (*this)(tpl::forward_tuple(std::forward<X>(x)),
                   tpl::forward_tuple(std::forward<Y>(y)...));
  }
};

/// Creates a unary composition: ucompose(f,g)(x) = f(g(x))
constexpr auto ucompose = pipe(MakeT<UComposition>{}, multary, lassoc);

/// Creates a generic composition.
///
/// compose(f,g)({x...}, {y...}) = f(g(x...), y...)
constexpr auto compose = pipe(MakeT<Composition>{}, multary, lassoc);

struct proj_f {
  template<class F, class ProjF, class...X,
           class = enable_if_t<(sizeof...(X) > 0)>>
  constexpr auto operator() (F&& f, const ProjF& pf, X&&...x) const &
    -> std::result_of_t<F(std::result_of_t<const ProjF&(X)>...)>
  {
    return invoke(std::forward<F>(f),
                  invoke(pf, std::forward<X>(x))...);
  }
};

struct lproj_f {
  template<class F, class ProjF, class X, class Y>
  constexpr auto operator() (F&& f, ProjF&& pf, X&& x, Y&& y) const
    -> std::result_of_t<F(std::result_of_t<ProjF(X)>, Y)>
  {
    return invoke(std::forward<F>(f),
                  invoke(std::forward<ProjF>(pf),
                         std::forward<X>(x)),
                  std::forward<Y>(y));
  }
};

struct rproj_f {
  template<class F, class ProjF, class X, class Y>
  constexpr auto operator() (F&& f, ProjF&& pf, X&& x, Y&& y) const
    -> std::result_of_t<F(X, std::result_of_t<ProjF(Y)>)>
  {
    return invoke(std::forward<F>(f),
                  std::forward<X>(x),
                  invoke(std::forward<ProjF>(pf),
                         std::forward<Y>(y)));
  }
};

/// proj(f,pf) -- constructs a projection, p, such that
///   p(x,y) <=> f(pf(x), pf(y))
///   p(x)   <=> ucompose(f, pf)(x)
constexpr auto proj = multary_n<2>(proj_f{});

/// lproj(f, pf, x, y) <=> f(pf(x), y)
constexpr auto lproj = multary_n<2>(lproj_f{});

/// rproj(f, pf, x, y) <=> f(x, pf(y))
constexpr auto rproj = multary_n<2>(rproj_f{});

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
  template<class F, class X>
  using R = std::result_of_t<F(X)>;

  template<class F, class Left, class Right, class X, class Y>
  constexpr auto operator() (F&& f, Left&& l, Right&& r, X&& x, Y&& y) const
    -> std::result_of_t<F(R<Left,X>, R<Right,Y>)>
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
  template<class Sig>
  using R = std::result_of_t<Sig>;

  template<class F, class Left, class Right, class X>
  constexpr auto operator() (F&& f, Left&& l, Right&& r, X& x) const
    -> std::result_of_t<F(R<Left(X&)>, R<Right(X&)>)>
  {
    return invoke(std::forward<F>(f),
                  invoke(std::forward<Left>(l), x),
                  invoke(std::forward<Right>(r), x));
  }
};

/// split(f,l,r)(x) <=> f(l(x), r(x))
constexpr auto split = multary_n<3>(split_f{});

template<template<class...>class Enabler, class F>
struct Enabled_f {
  F f;

  constexpr Enabled_f(F f) : f(std::move(f)) { }

  // FIXME: Should be able to take more than one argument, but compiler
  // complains.
  template<class X, class = enable_if_t<Enabler<X>::value>>
  constexpr auto operator() (X&& x) const
    -> std::result_of_t<const F&(X&&)>
  {
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
