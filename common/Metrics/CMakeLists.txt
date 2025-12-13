cmake_minimum_required(VERSION 3.22)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE FILEPATH "vcpkg toolchain file")
endif()

project(Metrics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(opentelemetry-cpp CONFIG REQUIRED)

add_library(Metrics STATIC src/tracy.cpp)

target_include_directories(Metrics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Metrics PUBLIC
    fmt::fmt
    spdlog::spdlog
    #opentelemetry_cpp::opentelemetry_cpp
    opentelemetry-cpp::sdk
    opentelemetry-cpp::metrics
    opentelemetry-cpp::trace
    opentelemetry-cpp::otlp_grpc_exporter
)
