#string_constant

C++ implementation of compile time evaluated strings.
This library provides basic compile time strings as well as features for `fmt` and `std::string_view`.

## Usage
The library can be included into your existing CMake project by using `git submodules` and adding the following lines to your `CMakeLists.txt`:
```CMake
add_subdirectory(string_constant)
target_link_libraries(${target_name} string_constant::string_constant)
```

If you are **not** using `git submodules` and want to use the fetch content feature of CMake, just add the following lines to your `CMakeLists.txt`:
```CMake
include(FetchContent)
FetchContent_Declare(
    string_constant
    GIT_REPOSITORY git@github.com:dominicpoeschko/string_constant.git
    GIT_TAG master
)
FetchContent_MakeAvailable(string_constant)
target_link_libraries(${target_name} string_constant::string_constant)
```

## Examples
To build the examples go into the [examples](examples) directory and run the following commands:

```bash
mkdir build
cd build
cmake ..
make
```

The most basic usage can be found in [examples/basic_usage.cpp](examples/basic_usage.cpp), which covers basic string manipulation as well as printing through `std::cout` and `fmt::print`.

```c++
#include "string_constant/format.hpp"
#include "string_constant/string_constant.hpp"
#include <fmt/format.h>
#include <iostream>


int main(){
    using namespace sc::literals;
    static constexpr auto foo = "foobar"_sc;
    static constexpr auto bar = "this is"_sc;
    static constexpr auto foobar = foo + " "_sc +  bar;

    static_assert(foobar == "foobar this is"_sc);
    
    std::cout << foo << " + " << bar << " = "
            << foobar << std::endl;
    
    fmt::print("{} + {} = {}\n", foo, bar, foobar);
    return 0;
}
```

For the usage with compile-time format and the usage with `std::string_view` see the other [examples](examples).

### fmt support

`string_constant/string_constant.hpp` includes only the standard library. Everything that needs fmt is in
`string_constant/format.hpp`: `fmt::formatter<sc::StringConstant>` (so `fmt::print("{}", "x"_sc)` works), the
compile-time formatting `SC_FORMAT("value {}", 32)` and `sc::make_arg`. Include it where a `StringConstant` is formatted
with fmt; without it that is a compile error (no formatter), never a silent change.

## Tests

```sh
cmake -S tests -B build && cmake --build build && cd build && ctest --output-on-failure
```

Two suites: `test_string_constant` builds without fmt and fails to compile if `string_constant.hpp`
reaches fmt (also with a system fmt installed); `test_fmt` covers `format.hpp`: the formatter, the
consteval `format()` helpers and `SC_FORMAT`. Most of the library is `consteval`, so most of the
suite is `static_assert`s - a failure shows up as a build error rather than a failing test.
