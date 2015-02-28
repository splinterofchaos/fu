
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
                ,&X::f ,&X::f_crr
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
  X x;
  assert(set(x) == REGULAR);
  assert(set(X::Const()) == CONST_RVALUE);

  // Use a ranked overload to dispatch between subtlety different values.
  constexpr auto mem_overload =
    // FIXME: since mem_fn(f) no longer perfect forwards the object parameter,
    // fu::overload should be usable for X's many member functions. But gcc
    // complains that this creates "ambiguous base classes". It does not have
    // this problem with fu::ranked_overload. Perhaps because of its use of
    // decltype?
#ifndef __clang__
    fu::ranked_overload;
#else
    fu::overload;
#endif
  constexpr auto mems = mem_overload( &X::f_r
                                    , &X::f_rr
                                    , &X::f_crr
                                    , &X::f_cr
                                    , &Y::y
                                    );

  assert(mems(x) == REF);

#ifdef __clang__
  // FIXME: GCC invalidly tries to call X::f_r (requires non-const ref). It
  // compiles, but the assertions fail at runtime.
  static_assert(mems(X()) == RVALUE, "");
  static_assert(mems(static_cast<const X&>(x)) == CONST_REF, "");
  static_assert(mems(X::Const()) == CONST_RVALUE, "");
#endif
} 
