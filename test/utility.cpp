
#include <fu/fu.h>

int main() {
  static_assert(fu::add(1)(2) == 3, "");
  static_assert(fu::add(1,2,3,4) == 10, "");
  static_assert(fu::sub(10, 3, 2) == 5, "");
  static_assert(fu::bit_or(1,2,4,8) == 0xf, "");
  static_assert(fu::lshift(1,1) == 2, "");
  static_assert(fu::eq(10, 10) == true, "");
  static_assert(fu::less(5, 10) == true, "");
  static_assert(fu::not_(false), "");
  static_assert(fu::neg(-1) == 1, "");
  static_assert(fu::and_(true, true, true), "");
  static_assert(!fu::and_(true, false, true), "");
  static_assert(fu::less(1,2,3,4,5), "");
  static_assert(fu::less(1)(2,3,4,5), "");
  static_assert(!fu::less(1,2,5,4,3), "");
  static_assert(!fu::less(5,4,3,2,1), "");
  static_assert(!fu::less(5)(4,3,2,1), "");
  static_assert(fu::eq(1,1,1,1), "");
  static_assert(!fu::eq(1,2,2,2), "");
}
