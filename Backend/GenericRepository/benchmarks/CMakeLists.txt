cmake_minimum_required(VERSION 3.22...3.29)
project(benchmarks LANGUAGES CXX)

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

add_executable(benchmarks
  benchmarks_query.cpp

  ../SQLiteDataBase.h
  ../GenericReposiroty.h
  ../../RedisCashe/RedisCache.h
  README.md
  benchmark_entity.cpp
  main.cpp
)

# # Redis++ static library
# add_library(redis++_static STATIC IMPORTED GLOBAL)
# set_target_properties(redis++_static PROPERTIES
#     IMPORTED_LOCATION "/opt/homebrew/lib/libredis++.a"
#     INTERFACE_INCLUDE_DIRECTORIES "/opt/homebrew/include"
# )

# # Hiredis static library
# add_library(hiredis_static STATIC IMPORTED GLOBAL)
# set_target_properties(hiredis_static PROPERTIES
#     IMPORTED_LOCATION "/opt/homebrew/lib/libhiredis.a"
#     INTERFACE_INCLUDE_DIRECTORIES "/opt/homebrew/include"
# )

target_link_libraries(benchmarks
    PRIVATE
    benchmark::benchmark
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt6::Sql
    redis++
    spdlog::spdlog
)

set_target_properties(benchmarks PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "@executable_path;@executable_path/../lib;/opt/homebrew/lib;/usr/local/lib"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
