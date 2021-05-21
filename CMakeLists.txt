cmake_minimum_required(VERSION 3.16)
PROJECT(utils-cpp)

option(UTILS_CPP_ENABLE_TESTS     "Enable tests for utils-cpp" OFF)
option(UTILS_CPP_ENABLE_BENCHMARK "Enable benchmark for utils-cpp" OFF)

FILE(GLOB_RECURSE SOURCES src/*.cpp src/*.h headers/*.h)

add_library(utils-cpp STATIC ${SOURCES})
set_property(TARGET utils-cpp PROPERTY CXX_STANDARD 17)
target_include_directories(utils-cpp PUBLIC headers)

add_subdirectory(tests)
