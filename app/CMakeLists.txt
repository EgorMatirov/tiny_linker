cmake_minimum_required(VERSION 3.9)
project(tiny_linker)

set(CMAKE_CXX_STANDARD 17)

add_executable(tiny_linker main.cpp)
target_link_libraries(tiny_linker PUBLIC tiny_linker_library)