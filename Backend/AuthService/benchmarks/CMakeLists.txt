cmake_minimum_required(VERSION 3.22)
project(authservice_benchmarks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)
FetchContent_MakeAvailable(benchmark)

link_directories(
    /usr/local/lib
    /opt/homebrew/lib
)

include(FetchContent)
FetchContent_Declare(
    libsodium
    GIT_REPOSITORY https://github.com/jedisct1/libsodium.git
    GIT_TAG        stable
)
FetchContent_MakeAvailable(libsodium)

add_executable(authservice_benchmarks
  #cqrs_benchmark.cpp
  README.md
)

target_include_directories(authservice_benchmarks PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../tests
    ${CMAKE_SOURCE_DIR}/Backend/AuthService/include
)

target_link_libraries(authservice_benchmarks PRIVATE
    AuthServiceCore
    benchmark::benchmark
    benchmark::benchmark_main
    sodium
    BackendMocks
)
