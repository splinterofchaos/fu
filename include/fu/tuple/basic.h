
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
  constexpr decltype(auto) operator() (F&& f, Tuple&& t,
                                       std::integer_sequence<I, N...>) const
  {
    return fu::invoke(std::forward<F>(f),
                      std::get<N>(std::forward<Tuple>(t))...);
  }

  template<class F, class Tuple>
  constexpr auto operator() (const F& f, Tuple&& t) const
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

} // namespace tpl
} // namespace fu
