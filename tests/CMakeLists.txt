cmake_minimum_required(VERSION 3.10)

project (test VERSION 1.0)

find_package(SEAL)
find_package(OpenMP)

add_subdirectory(googletest) # add googletest subdirectory

include_directories(googletest/include) # this is so we can #include <gtest/gtest.h>

add_executable(tests tests.cpp ../src/utils.cpp ../src/database.cpp ../src/pirutils.cpp)

target_link_libraries(tests gtest SEAL::seal OpenMP::OpenMP_CXX)
