

#include <fu/functional.h>
#include <fu/utility.h>

namespace fu {
namespace logic {

/// Logical function projection.
/// Invokes short-circuit logical operations on a predicate, p.
///
/// logical_project(ident,ok,p,x,y) <=> ok(p(x)) ? p(y) : ident
/// logical_project(false,id,p,x,y) <=> p(x) ? p(y) : false
/// logical_project(true,(!),p,x,y) <=> !p(x) ? p(y) : true
struct logical_project_f {
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

constexpr auto logical_project = multary_n<3>(logical_project_f{});

/// all(pred)(x....) <=> pred(x) && ...
constexpr auto all = logical_project(false, identity);

/// any(pred)(x...) <=> pred(x) || ...
constexpr auto any = logical_project(true, not_);

} // namespace logic
} // namespace fu
