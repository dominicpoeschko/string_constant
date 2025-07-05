#include "string_constant/string_constant.hpp"

#include <cassert>
#include <fmt/format.h>
#include <string_view>

int main() {
    using namespace sc::literals;
    static constexpr auto foobar = "foobar"_sc;

    std::string_view sv{foobar};
    std::string_view sv_cmp{"foobar"};
    assert(sv == sv_cmp);
    fmt::print("StringView: {}", sv);

    return 0;
}
