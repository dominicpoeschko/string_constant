cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(string_constant_examples VERSION 0.1.0)

find_package(fmt)

include_directories(../src)

add_executable(constant_string constant_string.cpp)
target_link_libraries(constant_string fmt::fmt-header-only)