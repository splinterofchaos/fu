
#include <fu/functional.h>

namespace fu {

/// Decorates a binary operation as multary and left-associative, and with an
/// identity element.
template<class F, class X>
constexpr auto numeric_binary(F f, X identity) {
  return pipe(overload(f, constant(identity)),
              multary, lassoc);
}


// Helper to define binary operations with identity elements.
#define DECL_BIN_OP(name, op, identity)                    \
  struct name##_f {                                        \
    template<class X, class Y>                             \
    constexpr auto operator() (X&& x, Y&& y) const         \
      -> decltype(std::declval<X>() op std::declval<Y>())  \
    { return std::forward<X>(x) op std::forward<Y>(y); }   \
  };                                                       \
  constexpr auto name = numeric_binary(name##_f{}, identity);

// Mathematical and arithmetic operators
DECL_BIN_OP(add,  +, 0);
DECL_BIN_OP(sub,  -, 0);
DECL_BIN_OP(mult, *, 1);
DECL_BIN_OP(div,  /, 1);
DECL_BIN_OP(rem,  %, 1);
DECL_BIN_OP(add_eq,  +=, 0);
DECL_BIN_OP(sub_eq,  -=, 0);
DECL_BIN_OP(mult_eq, *=, 1);
DECL_BIN_OP(div_eq,  /=, 1);
DECL_BIN_OP(rem_eq,  %=, 1);

DECL_BIN_OP(lshift,    <<,  0);
DECL_BIN_OP(rshift,    >>,  0);
DECL_BIN_OP(lshift_eq, <<=, 0);
DECL_BIN_OP(rshift_eq, >>=, 0);

// Logical operators
DECL_BIN_OP(or_,      ||, false);
DECL_BIN_OP(and_,     &&, true);
DECL_BIN_OP(xor_,     ^,  0);
DECL_BIN_OP(bit_or,  |,  false);
DECL_BIN_OP(xor_eq_,     ^=,  0);

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
constexpr auto bit_and = numeric_binary(bit_and_f{}, 0);

// Helper to define binary operations.
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

#undef DECL_BIN_OP
#undef DECL_REL_OP

} // namespace fu
