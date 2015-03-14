
#pragma once

#include <utility>
#include <functional>

#include <fu/basic.h>
#include <fu/tuple/basic.h>
#include <fu/iseq.h>

namespace fu {
namespace tpl {

constexpr struct concat_f {
  template<class...Tuple>
  constexpr auto operator() (Tuple&&...t) const {
    return std::tuple_cat(std::forward<Tuple>(t)...);
  }
} concat{};

/// A perfect-forwarding version of std::tuple_element
template<class Tuple, size_t I>
using Elem = decltype(std::get<I>(std::declval<Tuple>()));

template<size_t i, class F, class...Tuple>
constexpr auto applyI(F&& f, Tuple&&...t) {
  return invoke(f, std::get<i>(t)...);
}

template<size_t i>
struct applyI_f {
  template<class...X>
  constexpr auto operator() (X&&...x) const {
    return applyI<i>(std::forward<X>(x)...);
  }
};

struct apply_rows_f {
  template<size_t...i, class F, class...T>
  constexpr auto operator() (std::index_sequence<i...>, F&& f, T&&...t) const {
    return tuple(applyI<i>(f, std::forward<T>(t)...)...);
  }
};
constexpr auto apply_rows = multary(apply_rows_f{});

/// map(f, {x...}) = {f(x)...}
struct map_f {
  template<size_t...i, class F, class Tuple>
  constexpr auto do_map(std::index_sequence<i...>,
                        const F& f, Tuple&& t) const {

    return tuple(applyI<i>(f, std::forward<Tuple>(t))...);
  }
  template<size_t...i, class F, class Tuple, class...TupleB,
           class = std::enable_if_t<sizeof...(TupleB)>>
  constexpr auto do_map(std::index_sequence<i...>,
                        const F& f, Tuple&& t, TupleB&&...tb) const {
    // let gi = part(f, xi) where xi is the i'th element of the tuple, t.
    // let ti = map(gi, tb...)
    // Since map(g) returns a tuple, our result is obtained by concatenating
    // the result of each ti.
    using namespace std;
    return concat((*this)(closure(f, get<i>(forward<Tuple>(t))),
                          forward<TupleB>(tb)...)...);
  }

  template<size_t...i, class F, class...Tuple>
  constexpr auto operator() (std::index_sequence<i...> is,
                             const F& f, Tuple&&...t) const {
    return do_map(is, f, std::forward<Tuple>(t)...);
  }

  template<class F, class Tuple, class...Tpls>
  constexpr auto operator() (const F& f, Tuple&& t, Tpls&&...ts) const {
    using Size = std::tuple_size<std::decay_t<Tuple>>;
    return do_map(std::make_index_sequence<Size::value>{},
                  f,
                  std::forward<Tuple>(t),
                  std::forward<Tpls>(ts)...);
  }
};

constexpr auto map = multary(map_f{});

/// zip_with(f, {x,y,z}, {a,b,c}) = {f(x,a), f(y,b), f(z,c)}
struct zip_with_f {
  template<size_t...i, class F, class...T>
  constexpr auto do_zip(std::index_sequence<i...> is, F&& f, T&&...t) const {
    return apply_rows(is, f, std::forward<T>(t)...);
  }

  template<class F, class T, class...U>
  constexpr auto operator() (F&& f, T&& t, U&&...u) const {
    using Size = std::tuple_size<std::decay_t<T>>;
    return do_zip(std::make_index_sequence<Size::value>{},
                  std::forward<F>(f),
                  std::forward<T>(t),
                  std::forward<U>(u)...);
  }
};

constexpr auto zip_with = multary(zip_with_f{});
constexpr auto zip = zip_with(tuple);

/// ap({f,g,h}, {x,y,z}, {a,b,c}) = {f(x,a), g(y,b), h(z,c)}
struct ap_f {
  template<size_t...i, class Fs, class...Xs>
  constexpr auto operator() (std::index_sequence<i...> is,
                             Fs&& fs, Xs&&...xs) const {
    return apply_rows(is, invoke, fs, std::forward<Xs>(xs)...);
  }

  template<class Fs, class...Xs>
  constexpr auto operator() (Fs&& fs, Xs&&...xs) const {
    using Size = std::tuple_size<std::decay_t<Fs>>;
    return (*this)(std::make_index_sequence<Size::value>{},
                   std::forward<Fs>(fs),
                   std::forward<Xs>(xs)...);
  }
};

constexpr auto ap = multary(ap_f{});
// FIXME: Should be definable by this equation:
//constexpr auto ap = zip(invoke);

struct foldl_f {
  template<class F, class X, class Tuple, class I, I A>
  constexpr decltype(auto) operator() (const F& f, X&& acc, Tuple&& t,
                                       std::integer_sequence<I, A>) const {
    return invoke(f,
                  std::forward<X>(acc),
                  std::get<A>(std::forward<Tuple>(t)));
  }

  template<class F, class X, class Tuple, class I, I A, I...N,
           class = std::enable_if_t<(sizeof...(N) > 0)>>
  constexpr decltype(auto) operator() (const F& f, X&& acc, Tuple&& t,
                                       std::integer_sequence<I, A, N...>) const
  {
    return (*this)(f,
                   invoke(f,
                          std::forward<X>(acc),
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
    return invoke(f,
                  std::forward<X>(acc),
                  std::get<A>(std::forward<Tuple>(t)));
  }

  template<class F, class X, class Tuple, class I, I A, I...N,
           class = std::enable_if_t<(sizeof...(N) > 0)>>
  constexpr decltype(auto) operator() (const F& f, X&& acc, Tuple&& t,
                                 std::integer_sequence<I, A, N...>) const {
    return invoke(f,
                  (*this)(f, std::forward<X>(acc), std::forward<Tuple>(t),
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
    return map(iseq::take<Size::value - 1>(iseq::make(t)),
               identity, std::forward<Tuple>(t));
  }
} init{};

constexpr struct tail_f {
  /// tail({y, x...}) = {x...}
  template<class Tuple>
  constexpr auto operator() (Tuple&& t) const {
    return map(iseq::drop<1>(iseq::make(t)),
               identity, std::forward<Tuple>(t));
  }
} tail{};

constexpr struct rot_f {
  template<std::size_t...i, class Tuple>
  static constexpr decltype(auto) do_rot(std::index_sequence<0, i...>,
                                         Tuple&& t)
  {
    return forward_tuple(std::get<i>(std::forward<Tuple>(t))...,
                         std::get<0>(std::forward<Tuple>(t)));
  }

  template<class Tuple>
  constexpr decltype(auto) operator() (Tuple&& t) const {
    return do_rot(iseq::make(t), std::forward<Tuple>(t));
  }
} rot{};

constexpr struct rrot_f {
  template<std::size_t...i, class Tuple>
  static constexpr decltype(auto) do_rot(std::index_sequence<i...>,
                                         Tuple&& t)
  {
    static_assert(sizeof...(i) == size<Tuple>() - 1, "");
    return forward_tuple(std::get<size<Tuple>() - 1>(std::forward<Tuple>(t)),
                         std::get<i - 1>(std::forward<Tuple>(t))...);
  }

  template<class Tuple>
  constexpr decltype(auto) operator() (Tuple&& t) const {
    return do_rot(iseq::drop<1>(iseq::make(t)),
                  std::forward<Tuple>(t));
  }
} rrot{};

} // namespace tpl
} // namespace fu
