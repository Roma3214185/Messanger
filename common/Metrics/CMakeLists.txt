cmake_minimum_required(VERSION 3.22)
project(Metrics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

add_library(Metrics STATIC src/tracy.cpp)

target_include_directories(Metrics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Metrics PUBLIC
    fmt::fmt
    spdlog::spdlog

    /opt/homebrew/lib/libopentelemetry_trace.a
    /opt/homebrew/lib/libopentelemetry_common.a
    /opt/homebrew/lib/libopentelemetry_resources.a
    /opt/homebrew/lib/libopentelemetry_sdk.a
    /opt/homebrew/lib/libopentelemetry_exporter_otlp_grpc.a
)
