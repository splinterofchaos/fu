[![Build Status](https://travis-ci.org/splinterofchaos/fu.svg?branch=travis)](https://travis-ci.org/splinterofchaos/fu)

# fu
Functional Utilities for C++14.

Fu hopes to offer 
 * Utilities for defining functions in terms of other ones.
 * A set of transparent function objects for common operations like `add`, `eq`, etc.. 
 * Utilities for creating transparent function objects.

github: https://github.com/splinterofchaos/fu
biicode: http://www.biicode.com/splinterofchaos/fu
 
# Transparent Function Objects
It can be difficult in C++ to use template functions with higher order functions, like those defined in `<algorithm>`. For example:
```c++
#include <iostream>

template<class X>
void print(const X& x) {
  std::cout << x;
}

int main() {
  auto v = {1,2,3,4};
  std::for_each(std::begin(v), std::end(v), std::bind(print<int>, std::placeholders::_1));
}
```
Not only must we explicitely specify the type of `print`'s arguments to avoid ambiguity, but `std::bind`'s syntax is overly verbose and inconvenient. Using `fu`, we can write...
```c++
#include <algorithm>
#include <iostream>
#include <fu/utility.h>

auto print = fu::lshift(std::ref(std::cout));

int main() {
  auto v = {1,2,3,4,5};
  std::for_each(std::begin(v), std::end(v), print);
}
```
Here, `fu::lshift` is a function object representing `operator<<`, and by supplying just one argument, we create a partial function. By default, `fu` will copy arguments when constructing partial functions, but `std::ref` prevents that.

FU also provides several projection functions such as `proj`, `split`, and `join` for use with `<algorithm>`. For more information, see "[Common `<algorithm>` patterns](http://yapb-soc.blogspot.com/2015/02/common-algorithm-patterns.html)".

"Transparent function object" means a function object that represents an overload set or has a templated function call operator such that it can be sent to higher order functions, like `std::for_each`, without needing to specify the types of its arguments. fu supplies a set of function objects, mirroring those supplied in `<utility>`, like `add`, `sub`(tract), `less`, `eq`(ual), and more. Given one argument, they create a partial application. Given many, they apply the arguments from left to right. For exmple, `fu::add(1,2,3,4)` would be equivalent to writing `((1+2)+3)+4`, and `fu::add(1)(2)` would be equivalent to `fu::add(1,2)`. `fu::less(1,2,3)` is equivalent to `1 < 2 && 2 < 3`.

# Defining Functions in Terms of Other Ones
TODO

# Creating generic function objects.
A multary (or "multiple-arity") function is one that takes two or more arguments. `fu::multary` enables a function to return a partial application if given only one argument.
```c++
// GCD implementation from n4061
constexpr int gcd_impl(int a, int b) {
  return b ? gcd_impl(b, std::abs(a) % std::abs(b)) : std::abs(a);
}
constexpr auto gcd = fu::multary(gcd_impl);
constexpr auto gcd5 = gcd(5);
constexpr int five = gcd5(10);  // five == gcd(5, 10)
```
One might note that `gcd` is an associative operation; `gcd(x, y, z) == gcd(x, gcd(y, z))`. `fu::lassoc`  (implying "left associativity") allows it to work on an arbitrary number of arguments.
```c++
constexpr auto gcd = fu::lassoc(fu::multary(gcd_impl));
constexpr int five = gcd(5, 10, 15, 20);
```
Having many layers of nesting parenthesies can make code both harder to write and read. `fu::pipe` allows one to chain operations, silimar to how the ranges proposal by Erin Neibler uses piping.
```c++
constexpr auto gcd = fu::pipe(gcd_impl, fu::lassoc, fu::multary);
// Think of as: "gcd = gcd_impl | lassoc | multary;"
```
Note that most of fu's utilities are transparent function objects, so we can pass them directly to higher order functions without ambiguity.
