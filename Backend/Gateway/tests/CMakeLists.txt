cmake_minimum_required(VERSION 3.16)
project(GatewayTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Test)
find_package(Catch2 3 REQUIRED)

add_executable(GatewayTests
    main.cpp
    test_gatewayserver.cpp
    test_middlewares.cpp
)

target_link_libraries(GatewayTests
    PRIVATE
        GatewayCore
        Catch2::Catch2
        Qt6::Test
        BackendMocks
        ProjectOptions
)

set_target_properties(GatewayTests PROPERTIES AUTOMOC ON)

include(CTest)
enable_testing()
include(Catch)
catch_discover_tests(GatewayTests)
