cmake_minimum_required(VERSION 3.16)
PROJECT(utils-cpp LANGUAGES CXX)

option(UTILS_CPP_ENABLE_TESTS     "utils-cpp: Enable tests" OFF)
option(UTILS_CPP_ENABLE_BENCHMARK "utils-cpp: Enable benchmark" OFF)

set(UTILS_CPP_GTEST_SEARCH_MODE "Auto" CACHE STRING "utils-cpp: Set GTest search mode")
set_property(CACHE UTILS_CPP_GTEST_SEARCH_MODE PROPERTY STRINGS "Auto" "Force" "Skip")

FILE(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.h headers/*.h)

add_library(utils-cpp STATIC ${SOURCES})
target_include_directories(utils-cpp PUBLIC headers)
target_compile_features(utils-cpp PUBLIC cxx_std_17)

if(MSVC)
    target_link_options(utils-cpp PRIVATE "/ignore:4221")
    set_target_properties(utils-cpp PROPERTIES STATIC_LIBRARY_OPTIONS "/ignore:4221")
    target_compile_options(utils-cpp PRIVATE "/WX")
else()
    target_compile_options(utils-cpp PRIVATE "-Werror")
endif()

add_subdirectory(tests)
