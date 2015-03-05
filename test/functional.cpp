
#include <cassert>
#include "fu/fu.h"

constexpr int add3(int x, int y, int z) {
  return x + y + z;
}

constexpr int add_half(int x, int y) {
  return (x + y) / 2;
}

constexpr int half(int x) { return x / 2; }

constexpr struct unfixed_pow2_f {
  template<class Rec>
#ifndef __clang__
  constexpr
#endif
  std::size_t operator() (Rec r, std::size_t x) const {
    return x ? 2 * r(x-1) : 1;
  }
} unfixed_pow2{};

constexpr int divide(int x, int y) {
    return x / y;
}

int main() {
  static_assert(fu::rassoc(divide, 10,2) == 10/2, "");
  static_assert(fu::rassoc(divide, 10,8,4) == 10/(8/4), "");
  static_assert(fu::rassoc(divide, 10,8,8,2) == 10/(8/(8/2)), "");
  static_assert(fu::rassoc(divide)(10,8,8,2) == 10/(8/(8/2)), "");

  // FIXME: gcc cannot evaluate some tests as constexpr, although clang can,
  // and it's vice versa for other tests.
#ifdef __clang__
# define CLANG_STATIC_ASSERT(expr) static_assert(expr, "")
# define GCC_STATIC_ASSERT(expr) assert(expr)
#else
# define CLANG_STATIC_ASSERT(expr) assert(expr)
# define GCC_STATIC_ASSERT(expr) static_assert(expr, "")
#endif
  using fu::inc;
  CLANG_STATIC_ASSERT((fu::ucompose(inc,inc,inc,inc)(1) == 5));
  CLANG_STATIC_ASSERT((fu::mcompose(inc,inc,inc,inc)(1) == 5));

  constexpr auto f = fu::compose(add3, add_half);

  using fu::tpl::tuple;
  static_assert(f(tuple(1,1), tuple(1,1)) == 3, "");
  static_assert(f(tuple(2,2), tuple(1,1)) == 4, "");

#ifdef __clang__
  // Assert that multary_n<n> can be passed to higher order functions.
  constexpr auto _add3 = fu::pipe(add3, fu::multary_n<2>);
  static_assert(_add3(1)(1)(1) == 3, "");
#endif

  constexpr auto add_inc_half = fu::mcompose(half, inc, add3);
  static_assert(add_inc_half(1,2,3) == (6+1)/2, "");

  static_assert(fu::rproj(fu::mult)(fu::add(1))(1,0) == 1, "");
  static_assert(fu::rproj(fu::mult, fu::add(1))(1,0) == 1, "");

  constexpr auto g = fu::compose_n<2>(add3, add_half);
  static_assert(g(1,1,1,1) == 3, "");
  static_assert(g(2,2,1,1) == 4, "");

  static_assert(fu::flip(g)(1,1,2,2) == 4, "");
  static_assert(fu::flip(g,1,1,2,2) == 4, "");

  static_assert(fu::flip(fu::sub, 5, 10) == 5, "flip(-,5,10) <=> 10 - 5");
  static_assert(fu::flip(fu::less, 5, 4, 3, 2, 1), "");

  constexpr auto pow2 = fu::fix(unfixed_pow2);
  GCC_STATIC_ASSERT(pow2(0) == 1);
  GCC_STATIC_ASSERT(pow2(1) == 2);
  GCC_STATIC_ASSERT(pow2(2) == 4);
  GCC_STATIC_ASSERT(pow2(3) == 8);

  static_assert(fu::rpart(fu::less, 10)(5), "");
  static_assert(fu::rpart(fu::less, 5, 6, 7)(2,3,4), "");
}
