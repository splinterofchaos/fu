
#include <fu/functional.h>

namespace fu {

//template<class Xs>
//struct Remap;
//
//template<class X>
//struct Remap<std::vector<X>> {
//  template<class Y>
//  using type = std::vector<Y>;
//};
//
//template<class X>
//struct Remap<std::vector<X>> {
//  template<class Y>
//  using type = std::vector<Y>;
//};

struct transform_f {
  template<class F, class Xs>
  Xs& operator() (const F& f, Xs& xs) const {
    for (auto& x : xs) x = f(x);
    return xs;
  }
};

constexpr auto transform = multary(transform_f{});

struct foldl_f {
  template<class F, class X, class Xs>
  constexpr X operator() (const F& f, X x0, Xs&& xs) const {
    for (auto it = std::begin(xs); it != std::end(xs); it++)
    x0 = f(x0, *it);
    return std::move(x0);
  }
};

constexpr auto foldl = multary(foldl_f{});

}
