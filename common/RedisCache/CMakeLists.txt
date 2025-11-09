cmake_minimum_required(VERSION 3.22)
project(RedisCache LANGUAGES CXX)

set(HIREDIS_INCLUDE_DIR /opt/homebrew/opt/hiredis/include)
set(HIREDIS_LIBRARY /opt/homebrew/opt/hiredis/lib/libhiredis.a) # <-- static
set(REDISPP_LIBRARY /opt/homebrew/lib/libredis++.a)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(RedisCache STATIC src/RedisCache.cpp)

target_include_directories(RedisCache PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${HIREDIS_INCLUDE_DIR}
    /opt/homebrew/include
)

target_link_libraries(RedisCache
    PUBLIC
        Metrics
        Qt6::Core
        ${REDISPP_LIBRARY}
        ${HIREDIS_LIBRARY}
)
