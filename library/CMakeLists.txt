cmake_minimum_required(VERSION 3.9)
project(tiny_linker_library)

set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_library(tiny_linker_library
        src/ObjectFile.cpp
        src/ObjectFileImpl.cpp
        src/ExecutableFile.cpp
        src/ExecutableFileImpl.cpp
        src/TextSection.cpp
        src/TextSectionImpl.cpp
        src/Linker.cpp
        src/LinkerImpl.cpp)

target_include_directories(tiny_linker_library PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${LLVM_INCLUDE_DIRS})
