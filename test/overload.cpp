
#include <fu/fu.h>

#include <cassert>
#include <iostream>
#include <string>

enum {
  INT, CHAR, FLOAT, DOUBLE,

  // Member functions
  REGULAR, REF, RVALUE,
  CONST_REF, CONST_RVALUE,
  MEMBER
};

struct X {
  constexpr int f() { return REGULAR; }
  constexpr int f_r() & { return REF; }
  constexpr int f_rr() && { return RVALUE; }
  constexpr int f_cr() const & { return CONST_REF; }
  constexpr int f_crr() const && { return CONST_RVALUE; }

  constexpr X() { }

  static constexpr const X Const() { return X{}; }
};

struct Y {
  int y = MEMBER;
};

constexpr int f(double ) { return DOUBLE; }

constexpr int g(float) { return FLOAT; }

struct Int {
  constexpr int operator() (int) const { return INT; }
};

struct Char {
  constexpr int operator() (char) const { return CHAR; }
};

template<class R, class X>
constexpr R app1(R(*f)(X), X&& x) {
  return f(std::forward<X>(x));
}

int main() {
  constexpr auto set =
    fu::overload(Int{} ,Char{}
                ,f ,g
#ifdef __clang__
                ,&X::f ,&X::f_crr
#endif
                );

  static_assert(set(0) == INT, "");
  static_assert(fu::part(set,0)() == INT, "");
  static_assert(set('x') == CHAR, "");
  static_assert(set(0.0f) == FLOAT, "");
  static_assert(set(0.0) == DOUBLE, "");


  using IsInt = fu::meta::Part<std::is_same, int>;
  using IsIntD = fu::meta::UCompose<IsInt::type, std::decay_t>;
  constexpr auto onInt =
    fu::overload(fu::enable_if_f<IsIntD::type>(Int{}) ,f);
  constexpr int zero = 0;
  static_assert(onInt(zero) == INT, "");
  static_assert(onInt(0.0) != INT, "");

  // BUG: gcc cannot deduce member function calls as constexpr.
#ifdef __clang__
  X x;
  static_assert(set(x) == REGULAR, "");
  static_assert(set(X::Const()) == CONST_RVALUE, "");

  // Use a ranked overload to dispatch between subtlety different values.
  constexpr auto mems = fu::ranked_overload(&X::f_r
                                           ,&X::f_rr
                                           ,&X::f_cr
                                           ,&X::f_crr
                                           ,&Y::y);

  static_assert(mems(x) == REF, "");
  static_assert(mems(X()) == RVALUE, "");
  static_assert(mems(static_cast<const X&>(x)) == CONST_REF, "");
  static_assert(mems(X::Const()) == CONST_RVALUE, "");
  static_assert(mems(Y()) == MEMBER, "");
#endif
} 
