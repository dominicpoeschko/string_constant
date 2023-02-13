cmake_minimum_required(VERSION 3.18)

project(string_constant VERSION 0.1.0)

add_library(string_constant INTERFACE)

target_include_directories(string_constant INTERFACE src)

add_library(string_constant::string_constant ALIAS string_constant)
