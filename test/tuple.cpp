
#include <iostream>
#include <array>

#include <fu/fu.h>

struct Double {
  template<class X>
  constexpr auto operator() (X x) const {
    return x * X(2);
  }
};

constexpr struct Add {
  template<class X, class Y>
  constexpr auto operator() (X x, Y y) const {
    return x + y;
  }
} add{};

struct Int {
  int x;
  constexpr Int(int x) : x(x) { }
  constexpr int times(int y) const { return x * y; }
};

int main() {
  using namespace fu::tpl;
  constexpr int one = 1;
  constexpr auto t1 = tuple(one, 2.5);

  using T1 = const std::tuple<int, double>;

  static_assert(std::is_same<decltype(t1), T1>::value, "");
  constexpr auto t2 = map(Double(), t1);
  static_assert(std::is_same<decltype(t2), T1>::value, "map changed the type");

  static_assert(_0(t2) == 2, "");
  static_assert(_1(t2) == 5.0, "");
  static_assert(last(t2) == 5.0, "");

  static_assert(map(fu::add, tuple(0,1), tuple(2,4)) == tuple(2,4,3,5), "");

  static_assert(apply(add, t2) == 7.0, "");
  static_assert(apply(add)(t2) == 7.0, "");
  static_assert(foldl(add, 0, t2) == 7.0, "");

  // TODO: Pull in the GCC_ and CLANG_STATIC_ASSERTs from
  // test/functional.cpp
#ifdef __clang__
  static_assert(apply(&Int::x, tuple(Int{1})) == 1, "");
  static_assert(apply(&Int::times, tuple(Int{2}, 5)) == 10, "");
  constexpr Int two{2};
  static_assert(apply(&Int::times, tuple(&two, 5)) == 10, "");
#endif

  // These lines cause gcc 4.9 to error and suggest sending a bug report.
  // TODO: Create minimal test case.
  static_assert(foldl(add)(0, t2) == 7.0, "");
  static_assert(fu::closure(foldr, add, 0)(t2) == 7.0, "");

  static_assert(foldl(add, t2) == 7.0, "");
  static_assert(foldr(add, 0, t2) == 7.0, "");
  static_assert(foldr(add, t2) == 7.0, "");
  static_assert(init(t2) == tuple(2), "");
  static_assert(tail(t2) == tuple(5.0), "");

  constexpr auto apTest = ap( tuple(fu::sub, fu::add)
                            , tuple(      3,       1)
                            , tuple(      2,      -1)
                            , tuple(      1,       1));
  static_assert(apTest == tuple(0,1), "");

  constexpr auto zipTest = zip_with(fu::add, tuple(0, 1), tuple(1, 2), tuple(2, 3));
  static_assert(zipTest == tuple(3,6), "");
  static_assert(zip(tuple(1,2), tuple(1,2)) == tuple(tuple(1,1), tuple(2,2)), "");

  int x = 1;
  auto ref = tuple(std::ref(x), std::cref(x));
  static_assert(std::is_same<decltype(ref), std::tuple<int&, const int&>>::value, "");

  //// std::forward_as_tuple works differently than tpl::forward_tuple by
  //// because forward_as_tuple does not work on constexpr literals/creates
  //// dangling references.
  //auto fwd1 = forward_tuple(5, x, std::move(t2), std::ref(x));
  //auto fwd2 = std::forward_as_tuple(5, x, std::move(t2), std::ref(x));
  //static_assert(std::is_same<decltype(fwd1), decltype(fwd2)>::value, "");

  const int cx = 1;
  auto tie1 = tie(x, cx);
  auto tie2 = std::tie(x, cx);
  static_assert(std::is_same<decltype(tie1), decltype(tie2)>::value, "");

  static_assert(rot(tuple(1,2,3)) == tuple(2,3,1), "");
  static_assert(rrot(tuple(1,2,3)) == tuple(3,1,2), "");
}
