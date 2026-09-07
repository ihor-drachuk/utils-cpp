FILE(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.h *.cpp)

set(PROJECT_TEST_NAME test-${PROJECT_NAME})

add_executable(${PROJECT_TEST_NAME} ${SOURCES})

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_TEST_NAME} gtest gmock_main Threads::Threads ${PROJECT_NAME} ${PROJECT_NAME}-test-common)
target_include_directories(${PROJECT_TEST_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)

add_test(NAME ${PROJECT_TEST_NAME}-runner COMMAND ${PROJECT_TEST_NAME})
