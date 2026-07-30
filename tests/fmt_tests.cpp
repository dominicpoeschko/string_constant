// Tests for the parts the header only defines when fmt is available: the fmt::formatter for
// StringConstant, the consteval format() helpers, ArgHolder, and SC_FORMAT.
//
// fmt is pulled in by string_constant.hpp itself (it includes it behind __has_include, wrapped in
// the warning pragmas), so this file does not include it directly.
#include "string_constant/string_constant.hpp"

#include <cstdint>
#include <cstdio>
#include <string>
#include <tuple>

using namespace sc::literals;

namespace {

int failures = 0;

#define CHECK_EQ(actual, expected, msg)                                     \
    do {                                                                    \
        auto const actual_   = (actual);                                    \
        auto const expected_ = std::string{expected};                       \
        if(actual_ != expected_) {                                          \
            std::printf("FAIL: %s: got \"%s\" expected \"%s\" (line %d)\n", \
                        msg,                                                \
                        actual_.c_str(),                                    \
                        expected_.c_str(),                                  \
                        __LINE__);                                          \
            ++failures;                                                     \
        }                                                                   \
    } while(0)

// consteval formatting: the format string and the arguments are both compile time, the result is a
// StringConstant again. This is what uc_log builds its log headers from.
static_assert(sc::detail::format<42,
                                 7>("{}-{}"_sc)
              == "42-7"_sc);
static_assert(sc::detail::format<std::uint32_t{1234},
                                 std::uint8_t{5}>("{}, {}"_sc)
              == "1234, 5"_sc);
static_assert(sc::detail::format<255>("{:#x}"_sc) == "0xff"_sc);
static_assert(sc::detail::format<'c'>("{}"_sc) == "c"_sc);
static_assert(sc::detail::format<true>("{}"_sc) == "true"_sc);
static_assert(sc::detail::format<-1>("{:>4}|"_sc) == "  -1|"_sc);

// SC_FORMAT takes its arguments as an expression instead of as template parameters
static_assert(SC_FORMAT("{}-{}",
                        1,
                        2)
              == "1-2"_sc);
static_assert(SC_FORMAT("{}",
                        0.5)
              == "0.5"_sc);
static_assert(SC_FORMAT("no args") == "no args"_sc);

}   // namespace

int main() {
    // formatter<StringConstant>: a StringConstant is formattable like a string, specs included
    CHECK_EQ(fmt::format("{}", "abc"_sc), "abc", "format StringConstant");
    CHECK_EQ(fmt::format("{}", ""_sc), "", "format empty StringConstant");
    CHECK_EQ(fmt::format("[{:>5}]", "ab"_sc), "[   ab]", "format with spec");
    CHECK_EQ(fmt::format("{}-{}", "a"_sc, "b"_sc), "a-b", "format several");

    // ArgHolder defers to the formatter of whatever the generator returns
    CHECK_EQ(fmt::format("{}", sc::make_arg([]() { return 42; })), "42", "ArgHolder int");
    CHECK_EQ(fmt::format("[{:>4}]", sc::make_arg([]() { return 42; })),
             "[  42]",
             "ArgHolder with spec");
    CHECK_EQ(fmt::format("{}", sc::make_arg([]() { return std::string_view{"str"}; })),
             "str",
             "ArgHolder string_view");

    // the consteval results are ordinary StringConstants, so they format too
    CHECK_EQ(fmt::format("{}", sc::detail::format<9>("n={}"_sc)), "n=9", "format of format");

    if(failures != 0) {
        std::printf("%d checks failed\n", failures);
        return 1;
    }
    std::printf("all checks passed\n");
    return 0;
}
