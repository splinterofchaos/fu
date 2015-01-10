
#include <iostream>

#include <array>
#include <vector>

#include <fu/meta.h>

struct Double {
  template<class X>
  constexpr auto operator() (X x) const {
    return x * X(2);
  }
};

template<class X> struct T{};

int main() {
  using namespace fu::meta;

  using Eq = BCompose<std::is_same, std::decay_t>;
  static_assert(ApplyT<Eq, int&, int&&>::value, "");

  using TI = Apply<Part<T>::type, int>;
  static_assert(ApplyT<Eq, TI, T<int>>::value, "");
}
