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

    "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_common.dylib"
    "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_trace.dylib"
    "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_exporter_otlp_grpc.dylib"
    "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_resources.dylib"
    "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_proto.dylib"
    "/opt/homebrew/opt/opentelemetry-cpp/lib/libopentelemetry_proto_grpc.dylib"
)

set_target_properties(Metrics PROPERTIES
    INSTALL_RPATH "/opt/homebrew/opt/opentelemetry-cpp/lib"
)
