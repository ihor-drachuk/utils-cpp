FILE(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp *.h)
add_library(${PROJECT_NAME}-test-common STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME}-test-common PUBLIC .)
target_compile_features(${PROJECT_NAME}-test-common PUBLIC cxx_std_17)
