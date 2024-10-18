file(GLOB SOURCES CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

foreach( testsourcefile ${SOURCES} )
    string( REPLACE ".cpp" "" testname ${testsourcefile} )

    find_package(Threads REQUIRED)

    add_executable( benchmark-${testname} ${testsourcefile} )
    target_link_libraries(benchmark-${testname} gtest benchmark Threads::Threads ${PROJECT_NAME})

    add_test(NAME benchmark-${testname}-runner COMMAND benchmark-${testname})
endforeach( testsourcefile ${APP_SOURCES} )
