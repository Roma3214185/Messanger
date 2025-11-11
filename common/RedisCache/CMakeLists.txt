cmake_minimum_required(VERSION 3.22)
project(RedisCache LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h PATHS /opt/homebrew/include)
find_library(HIREDIS_LIBRARY NAMES hiredis PATHS /opt/homebrew/lib)

find_path(REDISPP_INCLUDE_DIR sw/redis++/redis++.h PATHS $HOME/local/include)
find_library(REDISPP_LIBRARY NAMES redis++ PATHS $HOME/local/lib)

add_library(RedisCache STATIC src/RedisCache.cpp)

target_include_directories(RedisCache PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${HIREDIS_INCLUDE_DIR}
    ${REDISPP_INCLUDE_DIR}
)

target_link_libraries(RedisCache PUBLIC
    ${HIREDIS_LIBRARY}
    ${REDISPP_LIBRARY}
    Metrics
    Qt6::Core
)
