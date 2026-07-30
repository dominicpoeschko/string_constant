// Tests for everything that does not need fmt - the header is usable without it, so this
// translation unit deliberately never sees <fmt/format.h> (see tests/CMakeLists.txt).
//
// Most of the library is consteval, so most of the suite is static_asserts: they fail the build
// rather than the run. The main() checks cover what only exists at run time (streaming, comparing
// against a value the compiler does not know).
#include "string_constant/string_constant.hpp"

#include <compare>
#include <cstdio>
#include <sstream>
#include <string>
#include <string_view>

using namespace sc::literals;

namespace {

int failures = 0;

#define CHECK(cond, msg)                                        \
    do {                                                        \
        if(!(cond)) {                                           \
            std::printf("FAIL: %s (line %d)\n", msg, __LINE__); \
            ++failures;                                         \
        }                                                       \
    } while(0)

// storage / stringView
static_assert(sc::StringConstant<'a',
                                 'b',
                                 'c'>::storage.size()
              == 3);
static_assert(sc::StringConstant<'a',
                                 'b',
                                 'c'>::stringView
              == "abc");
static_assert(std::string_view{"abc"_sc} == "abc");
static_assert(std::string_view{""_sc}.empty());
static_assert(sc::StringConstant<>::storage.empty());

// the literal builds the same type as spelling the pack out
static_assert(std::is_same_v<decltype("abc"_sc),
                             sc::StringConstant<'a',
                                                'b',
                                                'c'>>);

// concatenation
static_assert("ab"_sc + "cd"_sc == "abcd"_sc);
static_assert(""_sc + "cd"_sc == "cd"_sc);
static_assert("ab"_sc + ""_sc == "ab"_sc);

// equality, both against another StringConstant and against anything string_view-like
static_assert("abc"_sc == "abc"_sc);
static_assert(!("abc"_sc == "abd"_sc));
static_assert(!("abc"_sc == "ab"_sc));
static_assert("abc"_sc == "abc");
static_assert(!("abc"_sc == "abd"));

// ordering
static_assert(("a"_sc <=> "b"_sc) == std::strong_ordering::less);
static_assert(("b"_sc <=> "a"_sc) == std::strong_ordering::greater);
static_assert(("a"_sc <=> "a"_sc) == std::strong_ordering::equal);
static_assert(("ab"_sc <=> "a"_sc) == std::strong_ordering::greater);
static_assert(("a"_sc <=> std::string_view{"b"}) == std::strong_ordering::less);

// create() from a string_view generator, and the SC_LIFT macro that wraps it
static_assert(sc::create([]() { return std::string_view{"lifted"}; }) == "lifted"_sc);
static_assert(SC_LIFT("lifted") == "lifted"_sc);

constexpr char const helloArray[] = "hello";
static_assert(SC_LIFT(helloArray) == "hello"_sc);

// create() from a std::string generator: transient allocation in a consteval context, copied into
// the char pack
static_assert(sc::create([]() { return std::string{"made"}; }) == "made"_sc);
static_assert(sc::create([]() { return std::string{}; }) == ""_sc);

// escape(). uc_log escapes braces in function names this way, so that the result stays usable as a
// format string. Also the regression test for a consteval failure that only shows up under
// -fno-delete-null-pointer-checks (which -fsanitize=undefined implies): the escaped string is built
// through a std::string, and constructing that from a pointer the compiler will not compare against
// null aborts the whole evaluation.
constexpr auto isBrace   = [](auto c) { return c == '{' || c == '}'; };
constexpr auto duplicate = [](auto c) { return c; };

static_assert(sc::escape("a{b}"_sc,
                         isBrace,
                         duplicate)
              == "a{{b}}"_sc);
static_assert(sc::escape("no braces"_sc,
                         isBrace,
                         duplicate)
              == "no braces"_sc);
static_assert(sc::escape(""_sc,
                         isBrace,
                         duplicate)
              == ""_sc);
static_assert(sc::escape("{"_sc,
                         isBrace,
                         duplicate)
              == "{{"_sc);
static_assert(sc::escape("{}"_sc,
                         isBrace,
                         duplicate)
              == "{{}}"_sc);

// the inserted character does not have to be the matched one, and it goes after the match
constexpr auto isVowel  = [](auto c) { return c == 'a' || c == 'e'; };
constexpr auto toDollar = [](auto) { return '$'; };
static_assert(sc::escape("bead"_sc,
                         isVowel,
                         toDollar)
              == "be$a$d"_sc);

}   // namespace

int main() {
    // stream insertion
    {
        std::ostringstream out;
        out << "hello "_sc << "world"_sc;
        CHECK(out.str() == "hello world", "operator<<");
    }

    // comparisons against values the compiler cannot see through
    {
        std::string const runtimeString{"abc"};
        CHECK("abc"_sc == runtimeString, "== std::string");
        CHECK(!("abd"_sc == runtimeString), "!= std::string");
        CHECK(("abc"_sc <=> std::string_view{runtimeString}) == std::strong_ordering::equal,
              "<=> std::string_view");
        CHECK(("abb"_sc <=> std::string_view{runtimeString}) == std::strong_ordering::less,
              "<=> orders");
    }

    // the implicit conversion is what most call sites actually use
    {
        std::string_view const view = "converted"_sc;
        CHECK(view == "converted", "operator std::string_view");
    }

    if(failures != 0) {
        std::printf("%d checks failed\n", failures);
        return 1;
    }
    std::printf("all checks passed\n");
    return 0;
}
