
#pragma once

#include <tuple>

#include <fu/iseq.h>
#include <fu/invoke.h>
#include <fu/make/make.h>

namespace fu {
namespace tpl {

/// apply(f, {x...}) = f(x...)
constexpr struct apply_f {
  template<size_t i, class Tuple>
  using Elem = decltype(std::get<i>(std::declval<Tuple>()));

  template<class F, class Tuple, class I, I...N>
  constexpr decltype(auto) operator() (std::integer_sequence<I, N...>,
                                       F&& f, Tuple&& t) const
  {
    return fu::invoke(std::forward<F>(f),
                      std::get<N>(std::forward<Tuple>(t))...);
  }

  template<class F, class Tuple>
  constexpr decltype(auto) invoke1(F&& f, Tuple&& t) const {
    using IS = decltype(iseq::make(t));
    return (*this)(IS{}, std::forward<F>(f), std::forward<Tuple>(t));
  }

  template<class F, class...Tuple>
  constexpr auto operator() (F&& f, Tuple&&...t) const
  {
    // TODO: don't use temporary tuple
    return invoke1(std::forward<F>(f), std::tuple_cat(std::forward<Tuple>(t)...));
  }

  // Since apply_f is used to define generic partial application, it must
  // define the partial application of itself with a single function.
  template<class F>
  struct apply_1_f {
    F f;
    constexpr apply_1_f(F f) : f(std::move(f)) { }

    template<class Tuple>
    constexpr decltype(auto) operator() (Tuple&& t) const {
      return apply_f{}(f, std::forward<Tuple>(t));
    }
  };

  template<class F>
  constexpr apply_1_f<F> operator() (F f) const {
    return {std::move(f)};
  }
} apply{};

/// Constructs a tuple, similar to std::make_tuple.
constexpr auto tuple = fu::MakeT<std::tuple>{};

/// Constructs a tuple of references, similar to std::tie.
constexpr auto tie = fu::TieT<std::tuple>{};

/// Constructs a tuple, similar to std::forward_as_tuple.
constexpr auto forward_tuple = fu::ForwardT<std::tuple>{};

template<class...X>
constexpr std::integral_constant<std::size_t, sizeof...(X)>
size(const std::tuple<X...>&) { return {}; }

template<class Tuple>
constexpr decltype(fu::tpl::size(std::declval<Tuple>()))
size() { return {}; }

template<std::size_t i>
struct get_f {
  template<class Tuple>
  constexpr decltype(auto) operator() (Tuple&& t) const {
    return std::get<i>(std::forward<Tuple>(t));
  }
};

template<std::size_t i>
struct rget_f {
  template<class Tuple>
  constexpr decltype(auto) operator() (Tuple&& t) const {
    return std::get<size<Tuple>() - i - 1>(std::forward<Tuple>(t));
  }
};

constexpr auto _0 = get_f<0>{};
constexpr auto _1 = get_f<1>{};
constexpr auto _2 = get_f<2>{};
constexpr auto _3 = get_f<3>{};
constexpr auto _4 = get_f<4>{};

constexpr auto last = rget_f<0>{};

} // namespace tpl
} // namespace fu
