
# Basic operations:

FU defines `fu::tuple` as equivalent to `std::make_tuple`, `fu::tie` as `std::tie`, and `fu::forward_tuple` as similar to `std::forward_as_tuple`. `fu::forward_tuple` allows passing temporaries to create a non-temporary `tuple`. In general, `fu` uses the name of a type for the function that constructs it rather than supplying many "make_" functions.

```c++
std::tuple<int, X, Y&> t = fu::tpl::tuple(1, x, std::ref(y));
std::tuple<int, X&, Y&> u = fu::tpl::forward_tuple(1, x, y);
std::tuple<X&, Y&> w = fu::tpl::tie(x, y);
```

To concatenate two tuples, one can use `fu::tpl::concat` as the polymorphic object form of `std::tuple_cat`.

```c++
using namespace fu::tpl;
std::tuple<int, int> t = concat(tuple(0), tuple(0));
```

`init` and `tail` can be used to remove the first or last element.

```c++
using namespace fu::tpl;
std::tuple<X, Y, Z> t(x, y, z);
std::tuple<X, Y> u = init(t);
std::tuple<Y, Z> v = tail(t);
```

# Application

```
 <x,y,z>       <x,y>       <x,y> <a,b>      <x,y> <a,b>
  | | |         | |         | |   | |        | |   | | 
   \|/          | |          \|   |/          \|   |/  
 apply(f)      map(f)         zip(f)         ap(<f,g>)
    |           | |           /   \           /     \
    |          /   \         /     \         |       |
f(x, y, z)  <f(x),f(y)>  <f(x,a),f(y,b)>  <f(x,a), g(y,b)>
```

N3915 proposes `std::apply`, however neither gcc nor clang supplies it. `fu::tpl::apply` serves the same purpose, but implements a polymorphic function object.

```c++
using fu::tpl::apply;
auto sum = apply(fu::add);

std::tuple<int, int> t(1,1);
auto x = sum(t);             // x == 2
auto y = apply(fu::add, t);  // y == 2
```

`fu::tpl::map` can be used to apply a function to each tuple element, and can be used on multiple tuples.

```c++
using namespace fu::tpl;

// t = <0, 2.0>
std::tuple<int, double> t = map(fu::add(1), tuple(1, 1.0));

// u = <0+0, 0+2.0, 2.0+0, 2.0+2.0> = <0, 2.0, 2.0, 4.0>
auto u = map(fu::add, t, t);
```

If a function returned a tuple, then `map(f)` would be a function returning atuple of tuples. `fu::tpl::concat` can be used to flatten the tuple.

```c++
using namespace fu::tpl;

std::tuple<X, X> t = tuple(x, x);
std::tuple<std::tuple<X>, std::tuple<X>> u = map(tuple, t);
std::tuple<X, X> v = apply(concat, u);

// More suscinctly:
auto v2 = apply(concat, map(tuple, tuple(x,x)));
```

`fu::tpl::zip` can be used to join two or more tuples over a given function.

```c++
using namespace fu::tpl;

using Vec = std::tuple<int, int>
auto a = tuple(x0, y0);
auto b = tuple(x1, y1);
auto dist = zip(fu::sub, b, a);
assert(dist == tuple(x1 - x0, y1 - y0));
```

`fu::tpl::foldl` and `::foldr` allow one to fold a tuple by a binary function.

```c++
using namespace fu::tpl;

auto sum = foldl(fu::add);
int five = sum(tuple(2, 2, 1));              // five  = 2 + (2+1)
int five2 = foldr(fu::add, tuple(2, 2, 1));  // five2 = (2+2) + 1
```



