#include "string_constant/string_constant.hpp"

#include <fmt/format.h>
#include <iostream>

int main() {
    using namespace sc::literals;

    static constexpr auto compileString = SC_FORMAT("CompileTime {}", 32);
    static_assert(compileString == "CompileTime 32"_sc);
    return 0;
}
