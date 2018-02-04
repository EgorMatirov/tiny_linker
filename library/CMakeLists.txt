cmake_minimum_required(VERSION 3.9)
project(tiny_linker_library)

set(CMAKE_CXX_STANDARD 17)

add_library(tiny_linker_library
        src/ObjectFile.cpp
        src/ObjectFileImpl.cpp
        src/ExecutableFile.cpp
        src/ExecutableFileImpl.cpp)

target_include_directories(tiny_linker_library PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
