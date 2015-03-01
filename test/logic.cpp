
#include <fu/logic.h>

constexpr struct basic_less_f {
  template<class X, class Y>
  constexpr bool operator() (const X& x, const Y& y) const {
    return x < y;
  }
} basic_less{};

int main() {
  using namespace fu::logic;

  constexpr auto larger_than_10 = fu::part(basic_less, 10);
  static_assert(all(larger_than_10, 11, 14, 20), "");
  static_assert(!all(larger_than_10, 11, 14, 9), "");
  static_assert(!all(larger_than_10, 1, 14, 11), "");

  static_assert(any(larger_than_10, 11, 14, 20), "");
  static_assert(any(larger_than_10, 11, 4, 9), "");
  static_assert(!any(larger_than_10, 1, 4, 5), "");
}
