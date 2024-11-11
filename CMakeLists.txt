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

find_package(Threads REQUIRED)
target_link_libraries(utils-cpp PUBLIC Threads::Threads)

# Detect OS
if (WIN32)
    set(UTILS_CPP_OS_WINDOWS TRUE)
    target_compile_definitions(utils-cpp PUBLIC UTILS_CPP_OS_WINDOWS=1)

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(UTILS_CPP_OS_MACOS TRUE)
    target_compile_definitions(utils-cpp PUBLIC UTILS_CPP_OS_MACOS=1)

elseif(UNIX)
    set(UTILS_CPP_OS_LINUX TRUE)
    target_compile_definitions(utils-cpp PUBLIC UTILS_CPP_OS_LINUX=1)
endif()

# Detect compiler
if (MSVC)
    set(UTILS_CPP_COMPILER_MSVC TRUE)
    target_compile_definitions(utils-cpp PUBLIC UTILS_CPP_COMPILER_MSVC=1)

elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(UTILS_CPP_COMPILER_GCC TRUE)
    target_compile_definitions(utils-cpp PUBLIC UTILS_CPP_COMPILER_GCC=1)

elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(UTILS_CPP_COMPILER_CLANG TRUE)
    target_compile_definitions(utils-cpp PUBLIC UTILS_CPP_COMPILER_CLANG=1)

    if (CMAKE_CXX_COMPILER_ID MATCHES "Apple")
        set(UTILS_CPP_COMPILER_APPLE_CLANG TRUE)
        target_compile_definitions(utils-cpp PUBLIC UTILS_CPP_COMPILER_APPLE_CLANG=1)
    endif()
endif()

# Tune flags
if(UTILS_CPP_COMPILER_MSVC)
    target_link_options(utils-cpp PRIVATE "/ignore:4221")
    set_target_properties(utils-cpp PROPERTIES STATIC_LIBRARY_OPTIONS "/ignore:4221")
    target_compile_options(utils-cpp PRIVATE "/WX")
else()
    target_compile_options(utils-cpp PRIVATE "-Werror")
endif()

add_subdirectory(tests)
