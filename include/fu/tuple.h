
#pragma once

#include <utility>
#include <functional>

#include <fu/basic.h>
#include <fu/iseq.h>

namespace fu {
namespace tpl {

/// Constructs a tuple, similar to std::make_tuple.
constexpr auto tuple = fu::MakeT<std::tuple>{};

/// Constructs a tuple of references, similar to std::tie.
constexpr auto tie = fu::TieT<std::tuple>{};

/// Constructs a tuple, similar to std::forward_as_tuple.
constexpr auto forward_tuple = fu::ForwardT<std::tuple>{};

template<class...X>
constexpr size_t size(const std::tuple<X...>&) {
  return sizeof...(X);
}

/// A perfect-forwarding version of std::tuple_element
template<class Tuple, size_t I>
using Elem = decltype(std::get<I>(std::declval<Tuple>()));

/// map(f, {x...}) = {f(x)...}
struct map_f {
  template<class F, class Tuple, class I, I...N>
  constexpr auto operator() (const F& f, Tuple&& t,
                             std::integer_sequence<I, N...>) const {
    return tuple(f(std::get<N>(std::forward<Tuple>(t)))...);
  }

  template<class F, class Tuple>
  constexpr auto operator() (const F& f, Tuple&& t) const {
    return (*this)(f, std::forward<Tuple>(t), iseq::make(t));
  }
};

constexpr auto map = multary(map_f{});

struct foldl_f {
  template<class F, class X, class Tuple, class I, I A>
  constexpr decltype(auto) operator() (const F& f, X&& acc, Tuple&& t,
                                       std::integer_sequence<I, A>) const {
    return f(std::forward<X>(acc), std::get<A>(std::forward<Tuple>(t)));
  }

  template<class F, class X, class Tuple, class I, I A, I...N,
           class = std::enable_if_t<(sizeof...(N) > 0)>>
  constexpr decltype(auto) operator() (const F& f, X&& acc, Tuple&& t,
                                       std::integer_sequence<I, A, N...>) const
  {
    return (*this)(f,
                   f(std::forward<X>(acc),
                     std::get<A>(std::forward<Tuple>(t))),
                   std::forward<Tuple>(t),
                   std::integer_sequence<I, N...>{});
  }

  /// foldl(f, x, {a,b,c}) = f(f(f(x,a), b), c)
  template<class F, class X, class Tuple>
  constexpr decltype(auto) operator() (const F& f, X&& acc, Tuple&& t) const {
    return (*this)(f, std::forward<X>(acc), std::forward<Tuple>(t),
                   iseq::make(t));
  }

  /// foldl(f, {a, b, c}) = f(f(a,b), c)
  template<class F, class Tuple>
  constexpr decltype(auto) operator() (const F& f, Tuple&& t) const {
    return (*this)(f, std::get<0>(std::forward<Tuple>(t)),
                   std::forward<Tuple>(t), iseq::drop<1>(iseq::make(t)));
  }
};

constexpr auto foldl = multary(foldl_f{});

struct foldr_f {
  template<class F, class X, class Tuple, class I, I A>
  constexpr decltype(auto) operator() (const F& f, X&& acc, Tuple&& t,
                                 std::integer_sequence<I, A>) const {
    return f(std::forward<X>(acc), std::get<A>(std::forward<Tuple>(t)));
  }

  template<class F, class X, class Tuple, class I, I A, I...N,
           class = std::enable_if_t<(sizeof...(N) > 0)>>
  constexpr decltype(auto) operator() (const F& f, X&& acc, Tuple&& t,
                                 std::integer_sequence<I, A, N...>) const {
    return f((*this)(f, std::forward<X>(acc), std::forward<Tuple>(t),
                     std::integer_sequence<I, N...>{}),
             std::get<A>(std::forward<Tuple>(t)));
  }

  /// foldr(f, x, {a,b,c}) = f(f(f(x,c), b), a)
  template<class F, class X, class Tuple>
  constexpr decltype(auto) operator() (const F& f, X&& acc, Tuple&& t) const {
    return (*this)(f, std::forward<X>(acc), std::forward<Tuple>(t),
                   iseq::make(t));
  }

  /// foldr(f, {a, b, c}) = f(f(c,b), a)
  template<class F, class Tuple>
  constexpr decltype(auto) operator() (const F& f, Tuple&& t) const {
    return (*this)(f, std::get<0>(std::forward<Tuple>(t)),
                   std::forward<Tuple>(t), iseq::drop<1>(iseq::make(t)));
  }
};

constexpr auto foldr = multary(foldr_f{});

constexpr struct init_f {
  /// init({x..., y}) = {x...}
  template<class Tuple>
  constexpr auto operator() (Tuple&& t) const {
    using Size = std::tuple_size<std::decay_t<Tuple>>;
    return map(identity, std::forward<Tuple>(t),
               iseq::take<Size::value - 1>(iseq::make(t)));
  }
} init{};

constexpr struct tail_f {
  /// tail({y, x...}) = {x...}
  template<class Tuple>
  constexpr auto operator() (Tuple&& t) const {
    return map(identity, std::forward<Tuple>(t),
               iseq::drop<1>(iseq::make(t)));
  }
} tail{};

} // namespace tpl
} // namespace fu
