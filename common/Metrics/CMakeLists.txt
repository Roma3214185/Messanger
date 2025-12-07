cmake_minimum_required(VERSION 3.22)
project(Metrics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)

find_library(OTEL_TRACE_LIB trace PATHS /opt/homebrew/lib NO_DEFAULT_PATH)
find_library(OTEL_COMMON_LIB common PATHS /opt/homebrew/lib NO_DEFAULT_PATH)
find_library(OTEL_RESOURCES_LIB resources PATHS /opt/homebrew/lib NO_DEFAULT_PATH)
#find_library(OTEL_SDK_LIB sdk PATHS /opt/homebrew/lib NO_DEFAULT_PATH)
find_library(OTEL_EXPORTER_LIB exporter_otlp_grpc PATHS /opt/homebrew/lib NO_DEFAULT_PATH)

add_library(Metrics STATIC src/tracy.cpp)

target_include_directories(Metrics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

include_directories(/opt/homebrew/include)

target_link_libraries(Metrics PUBLIC
    fmt::fmt
    spdlog::spdlog

    ${OTEL_TRACE_LIB}
    ${OTEL_COMMON_LIB}
    ${OTEL_RESOURCES_LIB}
   # ${OTEL_SDK_LIB}
    ${OTEL_EXPORTER_LIB}
)
