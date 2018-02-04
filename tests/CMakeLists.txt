cmake_minimum_required(VERSION 3.9)
project(tiny_linker_tests)

set(CMAKE_CXX_STANDARD 17)

add_executable(tiny_linker_tests main.cpp)

target_link_libraries(tiny_linker_tests gtest gtest_main tiny_linker_library)