cmake_minimum_required(VERSION 3.16)
PROJECT(utils-cpp)

FILE(GLOB_RECURSE SOURCES src/*.cpp src/*.h headers/*.h)

add_library(utils-cpp STATIC ${SOURCES})
set_property(TARGET utils-cpp PROPERTY CXX_STANDARD 17)
target_include_directories(utils-cpp PUBLIC headers)
