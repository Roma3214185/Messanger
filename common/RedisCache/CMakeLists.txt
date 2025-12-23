cmake_minimum_required(VERSION 3.22)
project(RedisCache LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

include(FetchContent)

FetchContent_Declare(
  hiredis
  GIT_REPOSITORY https://github.com/redis/hiredis
  GIT_TAG origin/master
  GIT_SHALLOW TRUE
  SOURCE_DIR _deps/hiredis
  # Make sure redis++ finds fetched content with its find_package()
  OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
  redis-plus-plus
  GIT_REPOSITORY https://github.com/sjanel/redis-plus-plus.git
  GIT_TAG bugfix/hiredis-fetch-configure-file-found
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(redis-plus-plus)

add_library(RedisCache STATIC src/RedisCache.cpp)

target_include_directories(RedisCache
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(RedisCache PUBLIC
    redis++_static
    Metrics
)
