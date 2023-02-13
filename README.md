#string_constant

C++ implementation of compile time evaluated strings.
This library provides basic compile time strings as well as features for `fmt` and `std::string_view`.

## Usage
The library can be included into your existing CMake project by adding the following lines to your `CMakeLists.txt`:
```CMake
add_subdirectory(string_constant)
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