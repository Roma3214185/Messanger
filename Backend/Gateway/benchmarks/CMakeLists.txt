cmake_minimum_required(VERSION 3.22)
project(gateway_benchmarks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)
FetchContent_MakeAvailable(benchmark)

add_executable(gateway_benchmarks
    request_responce_pattern_benchmark.cpp
)

target_include_directories(gateway_benchmarks PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../tests
    ${CMAKE_SOURCE_DIR}/Backend/Gateway/include
)

target_link_libraries(gateway_benchmarks PRIVATE
    GatewayCore
    benchmark::benchmark
    benchmark::benchmark_main
    BackendMocks
)
