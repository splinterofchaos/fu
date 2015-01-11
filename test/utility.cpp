
#include <fu/utility.h>

int main() {
  static_assert(fu::add(1)(2) == 3, "");
  static_assert(fu::add(1,2,3,4) == 10, "");
  static_assert(fu::sub(10, 3, 2) == 5, "");
  static_assert(fu::bit_or(1,2,4,8) == 0xf, "");
  static_assert(fu::lshift(1,1) == 2, "");
  static_assert(fu::eq(10, 10) == true, "");
  static_assert(fu::less(5, 10) == true, "");
}
