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