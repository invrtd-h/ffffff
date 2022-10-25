cmake_minimum_required(VERSION 3.22)
project(underscore_cpp)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-fmodules-ts)

add_executable(underscore_cpp main.cpp ffffff/package.hpp ffffff/debug_tools.h ffffff/classify.h ffffff/tmf.hpp ffffff/basic_ops.hpp ffffff/interfaces.hpp ffffff/overload.hpp ffffff/pipeline.hpp ffffff/multiargs.hpp ffffff/bind.hpp ffffff/utils.hpp ffffff/functors.hpp ffffff/monads.hpp tu_1.cpp tu_1.h ffffff/reducible.hpp ffffff/practice.hpp)
