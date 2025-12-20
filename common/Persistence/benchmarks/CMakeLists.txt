cmake_minimum_required(VERSION 3.22)
project(benchmarks_sub LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Sql)
find_package(spdlog REQUIRED)

include(FetchContent)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark tests" FORCE)
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
FetchContent_MakeAvailable(benchmark)

set(HIREDIS_ROOT "/opt/homebrew/opt/hiredis")
#set(REDIS_PLUS_PLUS_ROOT "/opt/homebrew/opt/redis-plus-plus")

include_directories(
    ${HIREDIS_ROOT}/include
    ${CMAKE_SOURCE_DIR}/Backend/Persistence/include
)

link_directories(
    ${HIREDIS_ROOT}/lib
)

add_executable(benchmarks
    ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks_query.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/benchmark_entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/redis_cache_benchmark.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/build_entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/preparedStatements_benchmark.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/batcher_benchmark.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thread_pool_benchmark.cpp
)

add_executable(latencies
    benchmark_latency.cpp
)

target_include_directories(benchmarks PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/Backend/Persistence/include)

target_include_directories(latencies PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/Backend/Persistence/include)

target_link_libraries(benchmarks PRIVATE
    RedisCache
    Persistence
    RabbitMQClient
    benchmark::benchmark
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    spdlog::spdlog
    redis++
    hiredis
)

target_link_libraries(latencies PRIVATE
    RedisCache
    Persistence
    RabbitMQClient
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    spdlog::spdlog
    redis++
    hiredis
)
