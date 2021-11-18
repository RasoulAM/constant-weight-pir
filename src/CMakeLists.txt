cmake_minimum_required(VERSION 3.10)

project (main VERSION 1.0)

find_package(SEAL)
find_package(OpenMP)

add_executable(main main.cpp PIRserver.cpp client.cpp utils.cpp pirutils.cpp database.cpp)
add_executable(benchmark_eq benchmark_eq.cpp utils.cpp)

target_link_libraries(main SEAL::seal OpenMP::OpenMP_CXX)
target_link_libraries(benchmark_eq SEAL::seal OpenMP::OpenMP_CXX)
