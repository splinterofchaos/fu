
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
  int x = MEMBER;

  constexpr int f() { return REGULAR; }
  constexpr int f_r() & { return REF; }
  constexpr int f_rr() && { return RVALUE; }
  constexpr int f_cr() const & { return CONST_REF; }
  constexpr int f_crr() const && { return CONST_RVALUE; }

  constexpr X() { }

  static constexpr const X Const() { return X(); }
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

template<class X, class Y>
using Same = std::is_same<X,Y>;

int main() {
  using fu::invoke;

  static_assert(invoke(f,0) == DOUBLE, "");
  static_assert(invoke(Int{}, 0) == INT, "");

  // Test passes as long as this compiles.
  constexpr X x;
  X nonConst;
  invoke(&X::f, nonConst);
  invoke(&X::f_r, nonConst);
  invoke(&X::f_rr, X());
  invoke(&X::f_cr, x);
  invoke(&X::f_crr, X::Const());
  invoke(&X::x, x);

  using Ref = decltype(invoke(&X::x, nonConst));
  using CRef = decltype(invoke(&X::x, x));
  using RVal = decltype(invoke(&X::x, X()));
  using CRVal = decltype(invoke(&X::x, std::move(x)));

  using PRef = decltype(invoke(&X::x, &nonConst));
  using PCRef = decltype(invoke(&X::x, &x));

  static_assert(Same<int&, Ref>::value, "");
  static_assert(Same<const int&, CRef>::value, "");
  static_assert(Same<int&&, RVal>::value, "");
  static_assert(Same<const int&&, CRVal>::value, "");
  static_assert(Same<PRef, Ref>::value, "");
  static_assert(Same<PCRef, CRef>::value, "");
} 
