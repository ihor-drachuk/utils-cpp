if (NOT TARGET gtest)
    # Download and unpack googletest at configure time
    set(INSTALL_GTEST OFF CACHE BOOL "Do not install gtest" FORCE)
    set(INSTALL_GMOCK OFF CACHE BOOL "Do not install gmock" FORCE)

    # Prevent GoogleTest from overriding our compiler/linker options
    # when building with Visual Studio
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    message("Preparing GTest download 1/2")

    configure_file(CMakeLists.txt.in
        "${CMAKE_CURRENT_BINARY_DIR}/googletest-download/CMakeLists.txt")

    message("Preparing GTest download 2/2")

    execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" -S "${CMAKE_CURRENT_BINARY_DIR}/googletest-download" -B "${CMAKE_CURRENT_BINARY_DIR}/googletest-download2" RESULT_VARIABLE download_status)

    if (NOT "${download_status}" STREQUAL "0")
        message(FATAL_ERROR "Preparing GTest download - failed")
    endif()

    message("GTest download...")

    execute_process(COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_CURRENT_BINARY_DIR}/googletest-download2" RESULT_VARIABLE download_status2)

    if (NOT "${download_status2}" STREQUAL "0")
        message(FATAL_ERROR "GTest download - failed")
    endif()

    message("GTest download done")

    # Add googletest directly to our build. This adds
    # the following targets: gtest, gtest_main, gmock
    # and gmock_main
    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build)
endif()

enable_testing()
add_subdirectory(test)
