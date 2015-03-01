
# fu::logic

`"fu/logic.h"` includes a number of functions that take advantage of the
properties of logical operations. Consider `fu::transitive(binary, join)`.
```c++
// transitive(b,j)(x,y,z) <=> j(b(x,y), b(y,z))
auto less = transitive(std::less<>{}, std::logical_and<>{});
less(x,y,z);  // computes and(less(x,y), less(y,z))
```
While this version of `less` is correct, `less(x,y)` and `less(y,z)` must both
be computed, even if `less(x,y)` returns false--but `less(x,y) && less(y,z)`
would short-circuit if this first one failed.

TODO: complete example

## logical_project(identity, eval, pred), all, and any

`logical_project` is an overly-generic function that will test every argument,
`x_i`, with `eval(pred(x_i))` and returns `identity` when false. With only one
argument, it returns `eval(x)`. As it happens, `all(f)` and `any(f)` can be
defined in terms of `logical_project`.
```c++
/// all(pred)(x....) <=> pred(x) && ...
constexpr auto all = logical_project(false, identity);

/// any(pred)(x...) <=> pred(x) || ...
constexpr auto any = logical_project(true, not_);

auto big = [](int x) { return x > 5; };
assert(any(big, 3, 4, 5, 6));
assert(all(big, 6, 7, 8, 9));
```
