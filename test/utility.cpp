
#include <fu/fu.h>

#include <list>

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

  static_assert(fu::max(0,4,2) == 4, "");
  static_assert(fu::min(0,4,2) == 0, "");

  {
    constexpr int xs[3] = {0,1,2};
    static_assert(fu::size(xs) == 3, "");
    static_assert(fu::index(1, xs) == 1, "");
    static_assert(fu::front(xs) == 0, "");
    static_assert(fu::back(xs) == 2, "");
  }

  {
    // Test passes as long as this compiles.
    std::list<int> xs;
    fu::push_back(1, xs);
    fu::push_front(1, xs);
    fu::ref(xs).get().front() = 2;
  }
}
