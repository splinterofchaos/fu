

#include <fu/functional.h>

namespace fu {
namespace logic {

constexpr struct basic_not_f {
  constexpr bool operator() (bool b) const { return !b; }
} basic_not{};

/// Logical function projection.
/// Invokes short-circuit logical operations on a predicate, p.
///
/// logic::project(ident,ok,p,x,y) <=> ok(p(x)) ? p(y) : ident
/// logic::project(false,id,p,x,y) <=> p(x) ? p(y) : false
/// logic::project(true,(!),p,x,y) <=> !p(x) ? p(y) : true
struct project_f {
  template<class Identity, class Ok, class Pred, class X>
  constexpr decltype(auto) operator() (const Identity&, const Ok&,
                                       Pred&& p, X&& x) const {
    return fu::invoke(std::forward<Pred>(p), std::forward<X>(x));
  }

  template<class Identity, class Ok, class Pred, class X, class...Y,
           class = enable_if_t<(sizeof...(Y) > 0)>>
  constexpr decltype(auto) operator() (Identity&& id, Ok&& ok,
                                       Pred&& p, X&& x, Y&&...y) const {
    return fu::invoke(ok, fu::invoke(p, std::forward<X>(x)))
      ? (*this)(std::forward<Identity>(id), std::forward<Ok>(ok),
                std::forward<Pred>(p), std::forward<Y>(y)...)
      : std::forward<Identity>(id);
  }
};

struct transitive_f {
  template<class Identity, class Ok, class Pred, class X, class Y>
  constexpr decltype(auto) operator() (const Identity&, const Ok&,
                                       Pred&& p, X&& x, Y&& y) const {
    return fu::invoke(std::forward<Pred>(p),
                      std::forward<X>(x), std::forward<Y>(y));
  }

  template<class Identity, class Ok, class Pred, class X, class Y, class...Z,
           class = enable_if_t<(sizeof...(Z) > 0)>>
  constexpr decltype(auto) operator() (Identity&& id, Ok&& ok,
                                       Pred&& p, X&& x, Y&& y, Z&&...z) const {
    return fu::invoke(ok, fu::invoke(p, std::forward<X>(x), y))
      ? (*this)(std::forward<Identity>(id), std::forward<Ok>(ok),
                std::forward<Pred>(p),
                std::forward<Y>(y), std::forward<Z>(z)...)
      : std::forward<Identity>(id);
  }
};

constexpr auto transitive = multary_n<4>(transitive_f{});
constexpr auto project = multary_n<3>(project_f{});

/// all(pred)(x....) <=> pred(x) && ...
constexpr auto all = fu::logic::project(false, identity);

/// any(pred)(x...) <=> pred(x) || ...
constexpr auto any = fu::logic::project(true, basic_not);

/// none(pred)(x...) <=> !pred(x) && ...
constexpr auto none = multary(mcompose(basic_not, any));

} // namespace logic
} // namespace fu
