cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(string_constant_examples VERSION 0.1.0)

find_package(fmt)

include_directories(../src)

add_executable(basic basic_usage.cpp)
target_link_libraries(basic fmt::fmt-header-only)

add_executable(compileTime compile_time_fmt.cpp)
target_link_libraries(compileTime fmt::fmt-header-only)

add_executable(stringView string_view.cpp)
target_link_libraries(stringView fmt::fmt-header-only)
