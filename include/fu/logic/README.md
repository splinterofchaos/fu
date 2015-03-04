
# fu::logic

`"fu/logic.h"` includes a number of functions that take advantage of the
properties of logical operations. Consider `fu::transitive(binary, join)`.
```c++
// transitive(b,j)(x,y,z) <=> j(b(x,y), b(y,z))
auto less = fu::transitive(std::less<>{}, std::logical_and<>{});
less(x,y,z);  // computes and(less(x,y), less(y,z))
```
While this version of `less` is correct, `less(x,y)` and `less(y,z)` must both
be computed, even if `less(x,y)` returns false--but `less(x,y) && less(y,z)`
would short-circuit if this first one failed. `transitive` is too generic to
make a good `less`.

`fu::logic::transitive` can be used to create short-circuit logical
evaluations, although its interface looks quite obscure.
```c++
auto less = fu::logic::transitive(false, identity, std::less<>{});
less(1,2,3);  // computes 1 < 2 && 2 < 3
less(3,2,1);  // computes 3 < 2 and stops
```
## logic::transitive

All of the relational operations from `"fu/utility.h"` define themselves by
`op = logical_transitive(identity, eval, predicate)`. `identity` represents the
identity element of the operation (`true` for `and` and `false` for `or`),
`eval` returns a boolean deciding whether to recurse or not, and `predicate` is
the actual function evaluating the arguments.

Given `op(a,b,c,d)`, first `eval(predicate(a,b))` will be computed. If false,
then it returns `identity`, or otherwise will compute `eval(predicate(b,c))`
and repeat until the argument list ends.

*(See above for code example.)*

## logical_project(identity, eval, pred), all, and any

`logical_project` is an overly-generic function that will test every argument,
`x_i`, with `eval(pred(x_i))` and returns `identity` when false. With only one
argument, it returns `eval(x)`. As it happens, `all(f)` and `any(f)` can be
defined in terms of `logic::project`.
```c++
/// all(pred)(x....) <=> pred(x) && ...
constexpr auto all = logic::project(false, identity);

/// any(pred)(x...) <=> pred(x) || ...
constexpr auto any = logic::project(true, not_);

auto big = [](int x) { return x > 5; };
assert(any(big, 3, 4, 5, 6));
assert(all(big, 6, 7, 8, 9));
```

## either, both

`fu::logic::either(p1,p2)` returns a predicate, `e` such that `e(x,y)` computes
`p1(x, y) || p2(x, y)`. `fu::logic::both` works the same way, but uses `and`.
