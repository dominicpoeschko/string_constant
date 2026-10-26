#pragma once
// The fmt half of string_constant: fmt::formatter<sc::StringConstant>, consteval formatting
// (sc::detail::format, SC_FORMAT) and ArgHolder/make_arg. string_constant.hpp itself needs no fmt.
#include "string_constant.hpp"

#include <string>
#include <string_view>
#include <tuple>

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-parameter"
    #pragma clang diagnostic ignored "-Wundefined-func-template"
    #pragma clang diagnostic ignored "-Wweak-vtables"
    #pragma clang diagnostic ignored "-Wextra-semi"
    #pragma clang diagnostic ignored "-Wextra-semi-stmt"
    #pragma clang diagnostic ignored "-Wmissing-noreturn"
    #pragma clang diagnostic ignored "-Wduplicate-enum"
    #pragma clang diagnostic ignored "-Wsign-conversion"
    #pragma clang diagnostic ignored "-Wshorten-64-to-32"
    #pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    #pragma clang diagnostic ignored "-Wswitch-enum"
    #pragma clang diagnostic ignored "-Wswitch-default"
    #pragma clang diagnostic ignored "-Wfloat-equal"
    #pragma clang diagnostic ignored "-Wreserved-macro-identifier"
    #pragma clang diagnostic ignored "-Wglobal-constructors"
    #pragma clang diagnostic ignored "-Wdocumentation"
    #pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif
#ifndef FMT_USE_LOCALE
    #define FMT_USE_LOCALE 0
#endif
#ifndef FMT_UNICODE
    #define FMT_UNICODE 0
#endif
#ifdef FMT_HEADER_ONLY
    #define SC_RE_ENABLE_HEADER_ONLY
    #undef FMT_HEADER_ONLY
#endif
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#ifdef SC_RE_ENABLE_HEADER_ONLY
    #define FMT_HEADER_ONLY
    #undef SC_RE_ENABLE_HEADER_ONLY
#endif
#ifdef __clang__
    #pragma clang diagnostic pop
#endif

template<char... Chars>
struct fmt::formatter<sc::StringConstant<Chars...>> : formatter<string_view> {
    template<typename FormatContext>
    constexpr auto format(sc::StringConstant<Chars...>,
                          FormatContext& ctx) const {
        return formatter<string_view>::format(std::string_view{sc::StringConstant<Chars...>{}},
                                              ctx);
    }
};

namespace sc {
namespace detail {
    // a StringConstant as fmt's compiled format string (fmt/compile.h): parsed at compile time, usable in a constant
    // expression
    template<char... FormatChars>
    struct FormatStr : fmt::compiled_string {
        using char_type [[maybe_unused]] = char;

        constexpr explicit operator fmt::basic_string_view<char>() const noexcept {
            return StringConstant<FormatChars...>::stringView;
        }
    };

    // the text of the format string with the values Args{}() gives (a tuple), as a StringConstant; the text is
    // exactly as long as fmt says (formatted_size), whatever its length
    template<typename Args,
             char... FormatChars>
    consteval auto format(StringConstant<FormatChars...>) {
        constexpr auto generator = []() {
            return std::apply(
              [](auto... args) {
                  std::string text(fmt::formatted_size(FormatStr<FormatChars...>{}, args...), '\0');
                  fmt::format_to(text.data(), FormatStr<FormatChars...>{}, args...);
                  return text;
              },
              Args{}());
        };

        return create(generator);
    }

    // the values as template arguments
    template<auto... Args,
             char... FormatChars>
    consteval auto format(StringConstant<FormatChars...> fmt) {
        return format<decltype([]() { return std::tuple{Args...}; })>(fmt);
    }
}   // namespace detail

template<typename ArgGenerator>
struct ArgHolder {};

template<typename ArgGenerator>
consteval auto makeArg(ArgGenerator) -> ArgHolder<ArgGenerator> {
    return {};
}

}   // namespace sc

template<typename ArgGenerator>
struct fmt::formatter<sc::ArgHolder<ArgGenerator>> : formatter<decltype(ArgGenerator{}())> {
    template<typename FormatContext>
    constexpr auto format(sc::ArgHolder<ArgGenerator>,
                          FormatContext& ctx) const {
        return formatter<decltype(ArgGenerator{}())>::format(ArgGenerator{}(), ctx);
    }
};

#define SC_FORMAT(fmt, ...)                                                                     \
    ::sc::detail::format<decltype([]() { return std::make_tuple(__VA_ARGS__); })>(SC_LIFT(fmt))
