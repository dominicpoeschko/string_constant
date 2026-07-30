cmake_minimum_required(VERSION 3.28)
project(string_constant_tests)

include(FetchContent)

FetchContent_Declare(
    cmake_helpers
    GIT_REPOSITORY https://github.com/dominicpoeschko/cmake_helpers.git
    GIT_TAG master)

FetchContent_MakeAvailable(cmake_helpers)

include(${cmake_helpers_SOURCE_DIR}/FindOrFetch.cmake)

option(FMT_INSTALL "" OFF)
find_or_fetch_package(
    fmt
    GIT_REPOSITORY
    https://github.com/fmtlib/fmt
    GIT_TAG
    12.2.0
    GIT_BRANCH
    master)

include(${cmake_helpers_SOURCE_DIR}/BuildOptions.cmake)

# string_constant.hpp sets these before it includes fmt, so a fmt compiled without them defines locale_ref and context
# differently than the test sees them - gcc reports it as an ODR violation at LTO link time. Building fmt with the same
# values keeps it to one definition. PUBLIC so the macros are already set when a consumer reaches the #ifndef in the
# header.
if(TARGET fmt)
    target_compile_definitions(fmt PUBLIC FMT_USE_LOCALE=0 FMT_UNICODE=0)
endif()

add_subdirectory(../ ${CMAKE_CURRENT_BINARY_DIR}/string_constant)

# Propagate stdlib/sanitizer flags into fetched fmt so libc++/debug-ABI builds link. INTERFACE libraries are skipped:
# they compile nothing, and the helpers set non-INTERFACE properties, which is an error on those targets.
foreach(dep_target fmt)
    if(TARGET ${dep_target})
        get_target_property(_imported ${dep_target} IMPORTED)
        get_target_property(_type ${dep_target} TYPE)
        if(NOT _imported AND NOT _type STREQUAL "INTERFACE_LIBRARY")
            target_add_stdlib_flags(${dep_target})
            target_add_stdlib_debug_flags(${dep_target})
            target_add_sanitizer_flags(${dep_target} PUBLIC)
            target_add_optimizer_flags(${dep_target} PUBLIC)
        endif()
    endif()
endforeach()

enable_testing()

function(string_constant_add_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE string_constant::string_constant)
    target_add_default_build_options(${name} PRIVATE)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# fmt is deliberately not linked here: the header switches on __has_include(<fmt/format.h>), so this target is also the
# check that everything outside that block builds with no fmt in sight.
string_constant_add_test(test_string_constant string_constant_tests.cpp)

string_constant_add_test(test_fmt fmt_tests.cpp)
target_link_libraries(test_fmt PRIVATE fmt::fmt)
