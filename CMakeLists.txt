cmake_minimum_required(VERSION 3.22)
project(underscore_cpp)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-fmodules-ts)

add_executable(underscore_cpp main.cpp underscore.h debug_tools.h deprecated.h)
