cmake_minimum_required(VERSION 3.22)
project(ThreadPool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_library(ThreadPool STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/threadpool.cpp
)

target_include_directories(ThreadPool PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
