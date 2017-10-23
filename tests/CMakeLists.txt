cmake_minimum_required(VERSION 3.6)
project(test_project)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp ../include/align_help.h ../src/align_help.cpp
    ../include/filters.h ../include/align.h ../src/align.cpp)

add_subdirectory(googletest)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ../include ../externals/EasyBMP/include)

enable_testing()

add_executable(test_project ${SOURCE_FILES})
target_link_libraries(test_project gtest gtest_main)
add_test(test1 test_project)
