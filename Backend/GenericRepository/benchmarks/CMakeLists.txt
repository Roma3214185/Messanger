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
    #${REDIS_PLUS_PLUS_ROOT}/include
    ${CMAKE_SOURCE_DIR}/Backend
)

link_directories(
    ${HIREDIS_ROOT}/lib
    #${REDIS_PLUS_PLUS_ROOT}/lib
)

add_executable(benchmarks
    benchmarks_query.cpp
    benchmark_entity.cpp
    main.cpp
    redis_cache_benchmark.cpp
    build_entity.cpp
    preparedStatements_benchmark.cpp
    batcher_benchmark.cpp
    thread_pool_benchmark.cpp

    #../SQLiteDatabase.cpp
)

add_executable(latencies
    benchmark_latency.cpp

    #../SQLiteDatabase.cpp
)

target_include_directories(benchmarks PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/Backend)
target_include_directories(latencies PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/Backend)

target_link_libraries(benchmarks PRIVATE
    RedisCache
    GenericRepository
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
    GenericRepository
    RabbitMQClient
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    spdlog::spdlog
    redis++
    hiredis
)
