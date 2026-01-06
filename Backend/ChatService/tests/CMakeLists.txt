cmake_minimum_required(VERSION 3.16)
project(ChatServiceTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network WebSockets Gui Test)
find_package(Catch2 3 REQUIRED)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(ChatServiceTests ${MY_SOURCES})

set_target_properties(ChatServiceTests PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
)

target_link_libraries(ChatServiceTests
    PRIVATE
        ChatServiceCore
        Catch2::Catch2
        Qt6::Widgets
        Qt6::Test
        Qt6::Network
        BackendMocks
        ProjectOptions
)

set_target_properties(ChatServiceTests PROPERTIES AUTOMOC ON)

include(CTest)
enable_testing()
include(Catch)
catch_discover_tests(ChatServiceTests)
