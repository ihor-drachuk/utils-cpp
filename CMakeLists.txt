cmake_minimum_required(VERSION 3.16)
PROJECT(utils-cpp LANGUAGES CXX)

option(UTILS_CPP_ENABLE_TESTS     "utils-cpp: Enable tests" OFF)
option(UTILS_CPP_ENABLE_BENCHMARK "utils-cpp: Enable benchmark" OFF)

set(UTILS_CPP_GTEST_SEARCH_MODE "Auto" CACHE STRING "utils-cpp: Set GTest search mode")
set_property(CACHE UTILS_CPP_GTEST_SEARCH_MODE PROPERTY STRINGS "Auto" "Force" "Skip")

set(UTILS_CPP_TBB "Auto" CACHE STRING "utils-cpp: Link TBB (Intel Threading Building Blocks)")
set_property(CACHE UTILS_CPP_TBB PROPERTY STRINGS "Auto" "ON" "OFF")

FILE(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.h headers/*.h)

add_library(utils-cpp STATIC ${SOURCES})
target_include_directories(utils-cpp PUBLIC headers)
target_compile_features(utils-cpp PUBLIC cxx_std_17)

find_package(Threads REQUIRED)

# ---- Find TBB (Intel Threading Building Blocks) ----
# On Windows, assume TBB is always available without explicit linking
# On Linux/MacOS, find and link TBB based on the option
set(UTILS_CPP_TBB_ENABLED FALSE)

if(WIN32)
    # On Windows, assume TBB is always enabled
    set(UTILS_CPP_TBB_ENABLED TRUE)
else()
    # On Linux/MacOS, handle TBB based on the option
    if(UTILS_CPP_TBB STREQUAL "ON")
        find_package(TBB REQUIRED)
        set(UTILS_CPP_TBB_ENABLED TRUE)
    elseif(UTILS_CPP_TBB STREQUAL "Auto")
        find_package(TBB QUIET)
        if(TBB_FOUND)
            set(UTILS_CPP_TBB_ENABLED TRUE)
        endif()
    endif()
    # If OFF or not found in Auto mode, UTILS_CPP_TBB_ENABLED remains FALSE
endif()

# Enable execution policy flag when TBB is available
if(UTILS_CPP_TBB_ENABLED)
    target_compile_definitions(utils-cpp PUBLIC UTILS_CPP_EXECUTION_POLICY_ENABLED=1)
endif()
# ---- ----

# Find macOS frameworks
if(APPLE)
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
    find_library(IOKIT_FRAMEWORK IOKit)
endif()

target_link_libraries(utils-cpp PUBLIC
    Threads::Threads
    # Windows
    $<$<PLATFORM_ID:Windows>:wbemuuid.lib>
    $<$<PLATFORM_ID:Windows>:ole32.lib>
    $<$<PLATFORM_ID:Windows>:oleaut32.lib>
    $<$<PLATFORM_ID:Windows>:setupapi.lib>
    $<$<PLATFORM_ID:Windows>:iphlpapi.lib>
    # macOS
    $<$<PLATFORM_ID:Darwin>:${COREFOUNDATION_FRAMEWORK}>
    $<$<PLATFORM_ID:Darwin>:${IOKIT_FRAMEWORK}>
    # TBB on Linux/MacOS when found
    $<$<BOOL:${UTILS_CPP_TBB_ENABLED}>:$<$<NOT:$<PLATFORM_ID:Windows>>:TBB::tbb>>
)

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

    # Check specifically for MinGW
    if (${CMAKE_SYSTEM_NAME} MATCHES "MINGW" OR
        ${CMAKE_SYSTEM_NAME} MATCHES "MSYS" OR
        ${CMAKE_SYSTEM_NAME} MATCHES "CYGWIN" OR
        ${CMAKE_SYSTEM_NAME} MATCHES "Windows" OR
        ${CMAKE_CXX_COMPILER} MATCHES "mingw")
        set(UTILS_CPP_COMPILER_MINGW TRUE)
        target_compile_definitions(utils-cpp PUBLIC UTILS_CPP_COMPILER_MINGW=1)
    endif()

elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(UTILS_CPP_COMPILER_CLANG TRUE)
    target_compile_definitions(utils-cpp PUBLIC UTILS_CPP_COMPILER_CLANG=1)

    if (CMAKE_CXX_COMPILER_ID MATCHES "Apple")
        set(UTILS_CPP_COMPILER_APPLE_CLANG TRUE)
        target_compile_definitions(utils-cpp PUBLIC UTILS_CPP_COMPILER_APPLE_CLANG=1)
    endif()
endif()

# Detect architecture
if(CMAKE_OSX_ARCHITECTURES)
    set(TARGET_ARCHITECTURE ${CMAKE_OSX_ARCHITECTURES})
else()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
        set(UTILS_CPP_ARCH "x86_64")

    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386|i686")
        set(UTILS_CPP_ARCH "x86")

    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7|armv7l|arm|aarch64")
        set(UTILS_CPP_ARCH "ARM")

    elseif(UTILS_CPP_COMPILER_GCC)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND DEFINED _M_ARM64)
            set(UTILS_CPP_ARCH "ARM64")
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4 AND DEFINED _M_ARM)
            set(UTILS_CPP_ARCH "ARM")
        endif()
    else()
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(UTILS_CPP_ARCH "x86_64")
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(UTILS_CPP_ARCH "x86")
        else()
            set(UTILS_CPP_ARCH "unknown")
        endif()
    endif()
endif()

if (UTILS_CPP_ARCH MATCHES "x86_64")
    set(UTILS_CPP_ARCH_X86_64 TRUE)
    target_compile_definitions(utils-cpp PUBLIC UTILS_CPP_ARCH_X86_64=1)

elseif (UTILS_CPP_ARCH MATCHES "x86")
    set(UTILS_CPP_ARCH_X86 TRUE)
    target_compile_definitions(utils-cpp PUBLIC UTILS_CPP_ARCH_X86=1)

elseif (UTILS_CPP_ARCH MATCHES "ARM|ARM64")
    set(UTILS_CPP_ARCH_ARM TRUE)
    target_compile_definitions(utils-cpp PUBLIC UTILS_CPP_ARCH_ARM=1)
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
