cmake_minimum_required(VERSION 3.22)
project(benchmarks LANGUAGES CXX)

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
set(REDIS_PLUS_PLUS_ROOT "/opt/homebrew/opt/redis-plus-plus")

include_directories(
    ${HIREDIS_ROOT}/include
    ${REDIS_PLUS_PLUS_ROOT}/include
)

link_directories(
    ${HIREDIS_ROOT}/lib
    ${REDIS_PLUS_PLUS_ROOT}/lib
)

add_executable(benchmarks
    benchmarks_query.cpp
    benchmark_entity.cpp
    main.cpp
    redis_cache_benchmark.cpp
    build_entity.cpp
    ../SQLiteDataBase.h
    ../GenericReposiroty.h
    ../../RedisCashe/RedisCache.h
    ../IEntityBuilder.h
    README.md
    preparedStatements_benchmark.cpp
)

target_link_libraries(benchmarks
    PRIVATE
        benchmark::benchmark
        Qt6::Core
        Qt6::Network
        Qt6::Sql
        spdlog::spdlog
        redis++
        hiredis
)

set_target_properties(benchmarks PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "@executable_path;@executable_path/../lib;/opt/homebrew/lib;/usr/local/lib"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
