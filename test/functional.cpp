
#include <cassert>
#include "fu/functional.h"
#include "fu/tuple.h"

// TODO: define this in "fu/utility.h"
constexpr struct inc_f {
  constexpr int operator() (int x) const {
    return x+1;
  }
} inc{};

constexpr int add3(int x, int y, int z) {
  return x + y + z;
}

constexpr int add_half(int x, int y) {
  return (x + y) / 2;
}

int main() {
  // FIXME: gcc cannot evaluate this test as constexpr, although clang can.
#ifdef __clang__
# define ASSERT(expr) static_assert(expr, "")
#else
# define ASSERT(expr) assert(expr)
#endif
  ASSERT((fu::ucompose(inc,inc,inc,inc)(1) == 5));

  constexpr auto f = fu::compose(add3, add_half);

  using fu::tpl::tuple;
  static_assert(f(tuple(1,1), tuple(1,1)) == 3, "");
  static_assert(f(tuple(2,2), tuple(1,1)) == 4, "");
}
