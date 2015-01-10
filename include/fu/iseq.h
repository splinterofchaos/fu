
#pragma once

#include <utility>
#include <functional>


namespace fu {
namespace iseq {

// TODO: implement std::index_sequence_for for C++11

template<class...>
struct IseqOf;

template<class...X>
struct IseqOf<std::tuple<X...>> {
  using type = std::index_sequence_for<X...>;
};

template<class T>
using IseqOf_t = typename IseqOf<typename std::decay<T>::type>::type;

// TODO: C++11 version
template<class T, class Iseq = IseqOf_t<T>>
constexpr Iseq make(const T&) {
  return Iseq{};
}

template<size_t X, class I, I...N,
         class = typename std::enable_if<(X == 0)>::type>
constexpr auto drop(std::integer_sequence<I, N...> i)
  -> decltype(i)
{
  return i;
}

template<size_t X, class I, I N, I...M,
         class = typename std::enable_if<(X > 0)>::type>
constexpr auto drop(std::integer_sequence<I, N, M...>)
  -> decltype(drop<X - 1>(std::integer_sequence<I, M...>{}))
{
  static_assert(X <= sizeof...(M) + 1, "Index too high.");
  return drop<X - 1>(std::integer_sequence<I, M...>{});
}

template<size_t X, class I, I...N, I...M,
         class = typename std::enable_if<(X == 0)>::type>
constexpr auto take(std::integer_sequence<I, N...> i,
                    std::integer_sequence<I, M...>)
  -> decltype(i)
{
  return i;
}

template<size_t X, class I, I...N, I M, I...Ms,
         class = typename std::enable_if<(X > 0)>::type>
constexpr auto take(std::integer_sequence<I, N...> i,
                          std::integer_sequence<I, M, Ms...> j)
  -> decltype(take<X - 1>(std::integer_sequence<I, N..., M>{}, drop<1>(j)))
{
  static_assert(X <= sizeof...(Ms) + 1, "Index too high.");
  return take<X - 1>(std::integer_sequence<I, N..., M>{}, drop<1>(j));
}

template<size_t X, class I, I...N>
constexpr auto take(std::integer_sequence<I, N...> i)
  -> decltype(take<X>(std::integer_sequence<I>{}, i))
{
  return take<X>(std::integer_sequence<I>{}, i);
}

} // namespace iseq
} // namespace fu
