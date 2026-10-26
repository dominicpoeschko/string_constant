#pragma once
#include <algorithm>
#include <array>
#include <compare>
#include <string>
#include <string_view>

// Tells clang that a returned reference or a constructed object refers to this argument, so its lifetime analysis
// can see a dangling use. Nothing on other compilers.
#if __has_cpp_attribute(clang::lifetimebound)
    #define SC_LIFETIMEBOUND [[clang::lifetimebound]]
#else
    #define SC_LIFETIMEBOUND
#endif

namespace sc {
template<char... Chars>
struct StringConstant {
    static constexpr std::array<char, sizeof...(Chars)> storage{Chars...};
    static constexpr std::string_view                   stringView{storage.data(), storage.size()};

    constexpr operator std::string_view() const { return stringView; }
};

StringConstant() -> StringConstant<>;

namespace literals {
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
#else
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
#endif
    template<typename CharT,
             CharT... Chars>
    consteval StringConstant<Chars...> operator""_sc() {
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
    consteval auto unpackInto(std::index_sequence<Indices...>)
      -> StringConstant<StringViewGenerator{}()[Indices]...> {
        return {};
    }

    template<typename StringViewGenerator,
             std::size_t... Indices>
    consteval auto unpackInto(StringViewGenerator generator,
                              std::index_sequence<Indices...>) {
        return StringConstant<generator()[Indices]...>{};
    }

}   // namespace detail

template<typename StringViewLikeGenerator>
    requires(std::convertible_to<std::remove_cvref_t<decltype(StringViewLikeGenerator{}())>,
                                 std::string_view>
             && !std::same_as<std::remove_cvref_t<decltype(StringViewLikeGenerator{}())>,
                              std::string>)
consteval auto create(StringViewLikeGenerator)
  -> decltype(detail::unpackInto<StringViewLikeGenerator>(
    std::make_index_sequence<StringViewLikeGenerator{}().size()>{})) {
    return {};
}

namespace detail {
    template<typename StringGenerator>
        requires(std::same_as<std::remove_cvref_t<decltype(StringGenerator{}())>, std::string>)
    struct FromStringGenerator {
        static constexpr auto storage{[]() {
            std::array<char, []() { return StringGenerator{}().size(); }()> localStorage;

            auto const str = StringGenerator{}();

            std::copy(str.begin(), str.end(), localStorage.begin());
            return localStorage;
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
         char... Chars>
constexpr Stream& operator<<(Stream& outputStream SC_LIFETIMEBOUND,
                             StringConstant<Chars...> const&) {
    outputStream << StringConstant<Chars...>::stringView;
    return outputStream;
}

template<char... CharsLhs,
         char... CharsRhs>
consteval auto operator+(StringConstant<CharsLhs...>,
                         StringConstant<CharsRhs...>) -> StringConstant<CharsLhs...,
                                                                        CharsRhs...> {
    return {};
}

template<char... CharsLhs,
         char... CharsRhs>
consteval auto operator==(StringConstant<CharsLhs...> const&,
                          StringConstant<CharsRhs...> const&) -> bool {
    return false;
}

template<char... Chars>
consteval auto operator==(StringConstant<Chars...> const&,
                          StringConstant<Chars...> const&) -> bool {
    return true;
}

template<char... CharsLhs,
         typename StringViewLike>
constexpr auto operator==(StringConstant<CharsLhs...> const&,
                          StringViewLike const& rightHandSide) -> bool
    requires(std::convertible_to<decltype(rightHandSide),
                                 std::string_view>)
{
    constexpr std::string_view lhs{StringConstant<CharsLhs...>{}};
    std::string_view const     rhs{rightHandSide};

    return rhs == lhs;
}

template<char... CharsLhs,
         char... CharsRhs>
consteval auto operator<=>(StringConstant<CharsLhs...> const&,
                           StringConstant<CharsRhs...> const&) -> std::strong_ordering {
    constexpr std::string_view lhs{StringConstant<CharsLhs...>{}};
    constexpr std::string_view rhs{StringConstant<CharsRhs...>{}};
    return static_cast<std::strong_ordering>(lhs.compare(rhs) <=> 0);
}

template<char... CharsLhs,
         typename StringViewLike>
constexpr auto operator<=>(StringConstant<CharsLhs...>,
                           StringViewLike const& rightHandSide)
    requires(std::convertible_to<decltype(rightHandSide),
                                 std::string_view>)
{
    constexpr std::string_view lhs{StringConstant<CharsLhs...>{}};
    std::string_view const     rhs{rightHandSide};
    return static_cast<std::strong_ordering>(lhs.compare(rhs) <=> 0);
}

template<typename ShouldEscape,
         typename EscapeWith,
         char... Chars>
consteval auto escape(StringConstant<Chars...>,
                      ShouldEscape,
                      EscapeWith) {
    constexpr auto generator = []() {
        // Not from stringView: that picks libstdc++'s (char const*, size) constructor, whose null
        // check gcc cannot fold under -fno-delete-null-pointer-checks (implied by
        // -fsanitize=undefined), which fails the whole consteval evaluation. Not from begin()/end()
        // either: for no Chars those are null pointers (bugprone-string-constructor).
        std::string string;
        string.append_range(StringConstant<Chars...>::storage);

        auto pos = string.begin();
        while(pos != string.end()) {
            pos = std::find_if(pos, string.end(), ShouldEscape{});

            if(pos != string.end()) {
                pos = std::next(string.insert(std::next(pos), EscapeWith{}(*pos)));
            }
        }
        return string;
    };

    return create(generator);
}

}   // namespace sc

#define SC_LIFT(arg) ::sc::create([]() { return std::string_view{arg}; })
// The fmt formatter, consteval formatting (sc::detail::format) and SC_FORMAT are in
// string_constant/format.hpp.
