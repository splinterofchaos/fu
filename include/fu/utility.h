
#pragma once

#include <fu/functional.h>

namespace fu {

/// Decorates a binary operation as multary and left-associative, and with an
/// identity element.
template<class F>
constexpr auto numeric_binary(F f) {
  // FIXME: Why does this not work when multary is applied before lassoc?
  return pipe(std::move(f), lassoc, multary);
}


// Helper to define binary operations with identity elements.
#define DECL_BIN_OP(name, op)                              \
  struct name##_f {                                        \
    template<class X, class Y>                             \
    constexpr auto operator() (X&& x, Y&& y) const         \
      -> decltype(std::declval<X>() op std::declval<Y>())  \
    { return std::forward<X>(x) op std::forward<Y>(y); }   \
  };                                                       \
  constexpr auto name = numeric_binary(name##_f{});

// Mathematical and arithmetic operators
DECL_BIN_OP(add,     +);
DECL_BIN_OP(sub,     -);
DECL_BIN_OP(mult,    *);
DECL_BIN_OP(div,     /);
DECL_BIN_OP(rem,     %);
DECL_BIN_OP(add_eq,  +=);
DECL_BIN_OP(sub_eq,  -=);
DECL_BIN_OP(mult_eq, *=);
DECL_BIN_OP(div_eq,  /=);
DECL_BIN_OP(rem_eq,  %=);

DECL_BIN_OP(lshift,    <<);
DECL_BIN_OP(rshift,    >>);
DECL_BIN_OP(lshift_eq, <<=);
DECL_BIN_OP(rshift_eq, >>=);

// Logical operators
DECL_BIN_OP(or_,      ||);
DECL_BIN_OP(and_,     &&);
DECL_BIN_OP(xor_,     ^);
DECL_BIN_OP(bit_or,   |);
DECL_BIN_OP(xor_eq_,  ^=);

// TODO: Why does the macro fail on bit_and?
//DECl_BIN_OP(bit_and, &,  true);
struct bit_and_f {
  template<class X, class Y>
  constexpr auto operator() (X&& x, Y&& y)
    -> decltype(std::declval<X>(x) & std::declval<Y>(y))
  {
    return std::forward<X>(x) & std::forward<Y>(y);
  }
};
constexpr auto bit_and = numeric_binary(bit_and_f{});

// Helper to define binary relations.
#define DECL_REL_OP(name, op)                              \
  struct name##_f {                                        \
    template<class X, class Y>                             \
    constexpr auto operator() (X&& x, Y&& y) const         \
      -> decltype(std::declval<X>() op std::declval<Y>())  \
    { return std::forward<X>(x) op std::forward<Y>(y); }   \
  };                                                       \
  constexpr auto name = multary(name##_f{});

// Relational operators
DECL_REL_OP(less,       <);
DECL_REL_OP(greater,    >);
DECL_REL_OP(eq,         ==);
DECL_REL_OP(neq,        !=);
DECL_REL_OP(less_eq,    <=);
DECL_REL_OP(greater_eq, >=);

// Helper to define unary operators.
#define DECL_UNARY(name, op)                               \
  constexpr struct name##_f {                              \
    template<class X>                                      \
    constexpr auto operator() (X&& x) const                \
      -> decltype(op std::declval<X>())                    \
    { return op std::forward<X>(x); }                      \
  } name{};

DECL_UNARY(pos,   +);
DECL_UNARY(neg,   -);
DECL_UNARY(not_,  !);
DECL_UNARY(deref, *);
DECL_UNARY(addr,  &);  // TODO: Use std::address_of().

#undef DECL_BIN_OP
#undef DECL_REL_OP
#undef DECL_UNARY

} // namespace fu
