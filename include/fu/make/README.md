
# General object construction:

The standard "make_" functions have generally all the same property.

```c++
std::tuple<int, X, Y&> t = std::make_tuple(1, x, std::ref(y));
std::pair<int, X&> p = std::make_pair(1, std::ref(x));
std::optional<int> o = std::make_optional(0);
```

The function takes template arguments (`f(X,Y)`) and produces its type based on
those arguments (`tuple<X,Y>`). `MakeT` generalizes this principle.

```c++
constexpr auto make_pair = MakeT<std::pair>{};
constexpr auto make_tuple = MakeT<std::tuple>{};

constexpr int one = 1;
constexpr std::pair<int, int&> p = make_pair(one, std::ref(one));
```

`MakeT` converts `std::reference_wrapper` objects into normal references, but
otherwise decays the type of its argument.

`TieT` and `ForwardT` work similarly. `TieT` constructs an object of references
based on its parameters and `ForwardT` passes the parameters exactly.

```c++
constexpr auto tie = TieT<std::tuple>{};
constexpr auto forward_tuple = ForwardT<std::tuple>{};

int x;
const int y;
std::tuple<int&, const int&> t = tie(x, y);

// Note: std::forward_as_tuple would return an std::tuple<int&&, int&>,
//       which would make this hold a reference to a temporary.
std::tuple<int, int&> u = forward_tuple(0, x);
```
