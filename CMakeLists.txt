cmake_minimum_required(VERSION 3.22)
project(underscore_cpp)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-fmodules-ts)

add_executable(underscore_cpp main.cpp include/ffffff.hpp include/debug_tools.h include/classify.h include/tmf.hpp include/basic_ops.hpp)
