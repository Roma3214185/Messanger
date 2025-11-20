cmake_minimum_required(VERSION 3.16)
project(ChatServiceTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network WebSockets Gui Test)
find_package(spdlog REQUIRED)
find_package(Catch2 3 REQUIRED)

add_executable(ChatServiceTests
    main.cpp
    test_chatserver.cpp
    mocks/MockChatManager.h
    test_controller.cpp
    test_chatmanager.cpp
)

set_target_properties(ChatServiceTests PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "/opt/homebrew/lib"
    INSTALL_RPATH_USE_LINK_PATH ON
)

target_link_libraries(ChatServiceTests
    PRIVATE
        ChatServiceCore
        Catch2::Catch2
        Qt6::Widgets
        Qt6::Test
        Qt6::Network
        BackendMocks
)

set_target_properties(ChatServiceTests PROPERTIES AUTOMOC ON)

include(CTest)
enable_testing()
include(Catch)
catch_discover_tests(ChatServiceTests)
