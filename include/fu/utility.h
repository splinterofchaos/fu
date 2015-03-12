
#pragma once

#include <fu/functional.h>
#include <fu/logic.h>

namespace fu {

/// Decorates a binary operation as multary and left-associative, and with an
/// identity element.
template<class F>
constexpr auto numeric_binary(F f) {
  return pipe(std::move(f), lassoc, multary);
}


// Helper to define binary operations with identity elements.
#define DECL_BIN_OP(name, op)                              \
  struct name##_f {                                        \
    template<class X, class Y>                             \
    constexpr auto operator() (X&& x, Y&& y) const         \
      -> decltype(auto)                                    \
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
  constexpr decltype(auto) operator() (X&& x, Y&& y) const {
    return std::forward<X>(x) & std::forward<Y>(y);
  }
};
constexpr auto bit_and = numeric_binary(bit_and_f{});

// Helper to define unary operators.
#define DECL_UNARY(name, op)                               \
  constexpr struct name##_f {                              \
    template<class X>                                      \
    constexpr decltype(auto) operator() (X&& x) const      \
    { return op std::forward<X>(x); }                      \
  } name{};

DECL_UNARY(pos,   +);
DECL_UNARY(neg,   -);
DECL_UNARY(not_,  !);
DECL_UNARY(deref, *);
DECL_UNARY(addr,  &);  // TODO: Use std::address_of().

constexpr struct numeric_relational_f {
  template<class Binary, class Identity=bool, class Ok=identity_f>
  constexpr auto operator() (Binary b, Identity ident=false, Ok ok = Ok{}) const
  {
    return logic::transitive(ident, ok, b);
  }
} numeric_relational{};

// Helper to define binary relations.
#define DECL_REL_OP(name, op)                              \
  struct name##_f {                                        \
    template<class X, class Y>                             \
    constexpr auto operator() (X&& x, Y&& y) const         \
      -> decltype(auto)                                    \
    { return std::forward<X>(x) op std::forward<Y>(y); }   \
  };                                                       \
  constexpr auto name = numeric_relational(name##_f{});

// Relational operators
DECL_REL_OP(less,       <);
DECL_REL_OP(greater,    >);
DECL_REL_OP(eq,         ==);
DECL_REL_OP(neq,        !=);
DECL_REL_OP(less_eq,    <=);
DECL_REL_OP(greater_eq, >=);

#undef DECL_BIN_OP
#undef DECL_REL_OP
#undef DECL_UNARY

constexpr auto inc = add(1);

/// inc, but modifies its argument; returns a reference.
constexpr struct pre_inc_f {
  template<class Number> Number& operator() (Number& n) const {
    return ++n;
  }
} pre_inc{};

constexpr struct pre_dec_f {
  template<class Number> Number& operator() (Number& n) const {
    return --n;
  }
} pre_dec{};

/// inc, but modifies its argument; returns the previous value.
constexpr struct post_inc_f {
  template<class Number> Number operator() (Number& n) const {
    return n++;
  }
} post_inc{};

constexpr struct post_dec_f {
  template<class Number> Number operator() (Number& n) const {
    return n--;
  }
} post_dec{};

struct max_f {
  // FIXME: This should return decltype(auto), but GCC 4.9 deduces that the
  // call, max(1,2,3) returns a reference to temporary from Part::operator().
  // Changing Part::args to act like std::forward_as_tuple fixes this, but
  // makes the expression non-constexpr.
  template<class X, class Y>
  constexpr auto operator() (X&& x, Y&& y) const {
    return x < y ? std::forward<Y>(y) : std::forward<X>(x);
  }
};

struct min_f {
  template<class X, class Y>
  constexpr auto operator() (X&& x, Y&& y) const {
    return x < y ? std::forward<X>(x) : std::forward<Y>(y);
  }
};

constexpr auto max = multary(lassoc(max_f{}));
constexpr auto min = multary(lassoc(min_f{}));

constexpr struct size_f {
  template<class X, std::size_t N>
  constexpr std::size_t operator() (X (&)[N]) const {
    return N;
  }

  template<class X>
  constexpr auto operator() (const X& x) const {
    return x.size();
  }
} size{};

struct index_f {
  template<class Index, class X>
  constexpr decltype(auto) operator() (Index i, X&& x) const {
    return std::forward<X>(x)[i];
  }
};

constexpr auto index = multary(index_f{});

constexpr struct back_f {
  template<class X, std::size_t N>
  constexpr X& operator() (X (&arr)[N]) const {
    return arr[N-1];
  }

  template<class Container>
  constexpr decltype(auto) operator() (Container&& c) const {
    return std::forward<Container>(c).back();
  }
} back{};

constexpr struct front_f {
  template<class X>
  constexpr X& operator() (X* arr) const {
    return arr[0];
  }

  template<class Container>
  constexpr decltype(auto) operator() (Container&& c) const {
    return std::forward<Container>(c).front();
  }
} front{};

// TODO: While taking the element to insert first matches the functional style
// better, does it make sense for C++?

struct push_back_f {
  template<class X, class Container>
  Container&& operator() (X&& x, Container&& c) const {
    c.push_back(std::forward<X>(x));
    return std::forward<Container>(c);
  }
};

constexpr auto push_back = multary(push_back_f{});

struct push_front_f {
  template<class X, class Container>
  Container&& operator() (X&& x, Container&& c) const {
    c.push_front(std::forward<X>(x));
    return std::forward<Container>(c);
  }
};

constexpr auto push_front = multary(push_front_f{});

// TODO: emplace?

struct insert_f {
  template<class X, class Container>
  auto operator() (X&& x, Container&& c) const {
    return c.insert(std::forward<X>(x));
  }
};

constexpr auto insert = multary(insert_f{});

} // namespace fu
