cmake_minimum_required(VERSION 3.22)
project(Metrics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql Widgets Gui)
find_package(ZLIB REQUIRED)
find_package(spdlog)

find_library(PROM_CORE_LIB libprometheus-cpp-core.a PATHS /opt/homebrew/lib)
find_library(PROM_PULL_LIB libprometheus-cpp-pull.a PATHS /opt/homebrew/lib)

add_library(Metrics STATIC
    src/metrics.cpp
)

target_include_directories(Metrics
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Metrics
    ${PROM_CORE_LIB}
    ${PROM_PULL_LIB}
    ZLIB::ZLIB Qt6::Core Qt6::Widgets Qt6::Gui
    spdlog::spdlog
)
