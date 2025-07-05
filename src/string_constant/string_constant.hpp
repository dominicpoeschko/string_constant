#pragma once
#include <algorithm>
#include <array>
#include <compare>
#include <string>
#include <string_view>

#if __has_include(<fmt/format.h>)
    #ifdef __clang__
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wunused-parameter"
        #pragma clang diagnostic ignored "-Wundefined-func-template"
        #pragma clang diagnostic ignored "-Wweak-vtables"
        #pragma clang diagnostic ignored "-Wextra-semi"
        #pragma clang diagnostic ignored "-Wmissing-noreturn"
        #pragma clang diagnostic ignored "-Wduplicate-enum"
        #pragma clang diagnostic ignored "-Wsign-conversion"
        #pragma clang diagnostic ignored "-Wshorten-64-to-32"
    #endif
    #include <fmt/compile.h>
    #include <fmt/format.h>
    #include <fmt/ranges.h>
    #ifdef __clang__
        #pragma clang diagnostic pop
    #endif
#endif

namespace sc {
template<char... chars>
struct StringConstant {
    static constexpr std::array<char, sizeof...(chars)> storage{chars...};
    static constexpr std::string_view                   sv{storage.data(), storage.size()};

    constexpr operator std::string_view() const { return sv; }
};

namespace literals {
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
#else
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
#endif
    template<typename CharT,
             CharT... chars>
    consteval StringConstant<chars...> operator""_sc() {
        return {};
    }
#ifdef __clang__
    #pragma clang diagnostic pop
#else
    #pragma GCC diagnostic pop
#endif
}   // namespace literals

namespace detail {
    template<typename StringViewGenerator,
             std::size_t... Indices>
        requires(std::convertible_to<std::remove_cvref_t<decltype(StringViewGenerator{}())>,
                                     std::string_view>)
    consteval auto unpack_into(std::index_sequence<Indices...>)
      -> StringConstant<StringViewGenerator{}()[Indices]...> {
        return {};
    }

    template<typename StringViewGenerator,
             std::size_t... Indices>
    consteval auto unpack_into(StringViewGenerator x,
                               std::index_sequence<Indices...>) {
        return StringConstant<x()[Indices]...>{};
    }

}   // namespace detail

template<typename StringViewLikeGenerator>
    requires(std::convertible_to<std::remove_cvref_t<decltype(StringViewLikeGenerator{}())>,
                                 std::string_view>
             && !std::same_as<std::remove_cvref_t<decltype(StringViewLikeGenerator{}())>,
                              std::string>)
consteval auto create(StringViewLikeGenerator)
  -> decltype(detail::unpack_into<StringViewLikeGenerator>(
    std::make_index_sequence<StringViewLikeGenerator{}().size()>{})) {
    return {};
}

namespace detail {
    template<typename StringGenerator>
        requires(std::same_as<std::remove_cvref_t<decltype(StringGenerator{}())>, std::string>)
    struct FromStringGenerator {
        static constexpr auto storage{[]() {
            std::array<char, []() { return StringGenerator{}().size(); }()> local_storage;

            auto const str = StringGenerator{}();

            std::copy(str.begin(), str.end(), local_storage.begin());
            return local_storage;
        }()};

        static consteval auto sc() {
            return create([]() { return std::string_view{storage.begin(), storage.end()}; });
        }
    };
}   // namespace detail

template<typename StringGenerator>
    requires(std::same_as<std::remove_cvref_t<decltype(StringGenerator{}())>,
                          std::string>)
consteval auto create(StringGenerator) {
    return detail::FromStringGenerator<StringGenerator>::sc();
}

template<typename Stream,
         char... chars>
constexpr Stream& operator<<(Stream& os,
                             StringConstant<chars...> const&) {
    os << StringConstant<chars...>::sv;
    return os;
}

template<char... chars_lhs,
         char... chars_rhs>
consteval auto operator+(StringConstant<chars_lhs...>,
                         StringConstant<chars_rhs...>) -> StringConstant<chars_lhs...,
                                                                         chars_rhs...> {
    return {};
}

template<char... chars_lhs,
         char... chars_rhs>
consteval auto operator==(StringConstant<chars_lhs...> const&,
                          StringConstant<chars_rhs...> const&) -> bool {
    return false;
}

template<char... chars>
consteval auto operator==(StringConstant<chars...> const&,
                          StringConstant<chars...> const&) -> bool {
    return true;
}

template<char... chars_lhs,
         typename StringViewLike>
constexpr auto operator==(StringConstant<chars_lhs...> const&,
                          StringViewLike const& rhs_in) -> bool
    requires(std::convertible_to<decltype(rhs_in),
                                 std::string_view>)
{
    constexpr std::string_view lhs{StringConstant<chars_lhs...>{}};
    std::string_view const     rhs{rhs_in};

    return rhs == lhs;
}

template<char... chars_lhs,
         char... chars_rhs>
consteval auto operator<=>(StringConstant<chars_lhs...> const&,
                           StringConstant<chars_rhs...> const&) -> std::strong_ordering {
    constexpr std::string_view lhs{StringConstant<chars_lhs...>{}};
    constexpr std::string_view rhs{StringConstant<chars_rhs...>{}};
#if 0
return lhs <=> rhs;
#else
    return static_cast<std::strong_ordering>(lhs.compare(rhs) <=> 0);
#endif
}

template<char... chars_lhs,
         typename StringViewLike>
constexpr auto operator<=>(StringConstant<chars_lhs...>,
                           StringViewLike const& rhs_in)
    requires(std::convertible_to<decltype(rhs_in),
                                 std::string_view>)
{
    constexpr std::string_view lhs{StringConstant<chars_lhs...>{}};
    std::string_view const     rhs{rhs_in};
#if 0
return lhs <=> rhs
#else
    return static_cast<std::strong_ordering>(lhs.compare(rhs) <=> 0);
#endif
}

template<typename ShouldEscape,
         typename EscapeWith,
         char... chars>
consteval auto escape(StringConstant<chars...>,
                      ShouldEscape,
                      EscapeWith) {
    constexpr auto Generator = []() {
        std::string s{StringConstant<chars...>::sv};

        auto pos = s.begin();
        while(pos != s.end()) {
            pos = std::find_if(pos, s.end(), ShouldEscape{});

            if(pos != s.end()) {
                pos = std::next(s.insert(std::next(pos), EscapeWith{}(*pos)));
            }
        }
        return s;
    };

    return create(Generator);
}

}   // namespace sc

#if __has_include(<fmt/format.h>)
template<char... chars>
struct fmt::formatter<sc::StringConstant<chars...>> : formatter<string_view> {
    template<typename FormatContext>
    constexpr auto format(sc::StringConstant<chars...>,
                          FormatContext& ctx) const {
        return formatter<string_view>::format(std::string_view{sc::StringConstant<chars...>{}},
                                              ctx);
    }
};

namespace sc {
namespace detail {
    template<auto... args,
             char... formatChars>
    consteval auto format(StringConstant<formatChars...>) {
        constexpr auto Generator = []() {
            std::array<char, 1 << 20> storage;
            auto const                end
              = fmt::format_to(storage.data(),
                               FMT_COMPILE(std::string_view{StringConstant<formatChars...>{}}),
                               args...);
            return std::string{storage.data(), end};
        };

        return create(Generator);
    }

    template<typename Args,
             char... formatChars>
    consteval auto format(StringConstant<formatChars...>) {
        constexpr auto Generator = []() {
            std::array<char, 1 << 20> storage;
            auto                      call = [&](auto... args) {
                return fmt::format_to(
                  storage.data(),
                  FMT_COMPILE(std::string_view{StringConstant<formatChars...>{}}),
                  args...);
            };
            auto const end = std::apply(call, Args{}());
            return std::string{storage.data(), end};
        };

        return create(Generator);
    }
}   // namespace detail

template<typename ArgGenerator>
struct ArgHolder {};

template<typename ArgGenerator>
consteval auto make_arg(ArgGenerator) -> ArgHolder<ArgGenerator> {
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
#endif

#define SC_LIFT(arg) ::sc::create([]() { return std::string_view{arg}; })
#define SC_FORMAT(fmt, ...)                                                                     \
    ::sc::detail::format<decltype([]() { return std::make_tuple(__VA_ARGS__); })>(SC_LIFT(fmt))
