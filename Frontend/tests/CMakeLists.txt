cmake_minimum_required(VERSION 3.16)
project(FrontendServiceTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Catch2 REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network WebSockets Gui Test)
find_package(spdlog REQUIRED)

set(FRONTEND_SRC_DIR ${CMAKE_SOURCE_DIR}/Frontend/src)
set(DEBUG_PROFILING_DIR ${CMAKE_SOURCE_DIR}/DebugProfiling)

add_executable(FrontendServiceTests
    presenter_tests.cpp
    model_tests.cpp
    DataInputServices/DataInputServices_tests.cpp
)

target_include_directories(FrontendServiceTests PRIVATE
    ${FRONTEND_SRC_DIR}
    ${DEBUG_PROFILING_DIR}
    /usr/local/include
    /opt/homebrew/include
)

target_link_libraries(FrontendServiceTests PRIVATE
    FrontendLib
    Catch2::Catch2WithMain
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Network
    Qt6::WebSockets
    Qt6::Test
    spdlog::spdlog
)

include(CTest)
include(Catch)
catch_discover_tests(FrontendServiceTests)
