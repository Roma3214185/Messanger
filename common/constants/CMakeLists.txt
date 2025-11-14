cmake_minimum_required(VERSION 3.22)
project(Constants LANGUAGES CXX)

add_library(Constants INTERFACE)

target_include_directories(Constants INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
