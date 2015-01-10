
#include <iostream>
#include <array>

#include <fu/tuple.h>

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

int main() {
  using namespace fu::tpl;
  constexpr int one = 1;
  constexpr auto t1 = tuple(one, 2.5);

  using T1 = const std::tuple<int, double>;

  static_assert(std::is_same<decltype(t1), T1>::value, "");
  constexpr auto t2 = map(Double(), t1);
  static_assert(std::is_same<decltype(t2), T1>::value, "map changed the type");

  static_assert(std::get<0>(t2) == 2, "");
  static_assert(std::get<1>(t2) == 5.0, "");
  static_assert(apply(add, t2) == 7.0, "");
  static_assert(apply(add)(t2) == 7.0, "");
  static_assert(foldl(add, 0, t2) == 7.0, "");

  // These lines cause gcc 4.9 to error and suggest sending a bug report.
  // TODO: Create minimal test case.
  static_assert(foldl(add)(0, t2) == 7.0, "");
  static_assert(fu::closure(foldr, add, 0)(t2) == 7.0, "");

  static_assert(foldl(add, t2) == 7.0, "");
  static_assert(foldr(add, 0, t2) == 7.0, "");
  static_assert(foldr(add, t2) == 7.0, "");
  static_assert(init(t2) == tuple(2), "");
  static_assert(tail(t2) == tuple(5.0), "");

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
}
