cmake_minimum_required(VERSION 3.16)
project(NotificationServiceTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network WebSockets Gui Test)
find_package(spdlog REQUIRED)
find_package(Catch2 3 REQUIRED)

add_executable(NotificationServiceTests
    test_notificationmanager.cpp
    main.cpp
    mocks/MockConfigProvider.h
    mocks/MockSocket.h
    test_server.cpp
    mocks/MockNetworkManager.h
    #mocks/MockRabitMQClient.h
)

set_target_properties(NotificationServiceTests PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "/opt/homebrew/lib"
    INSTALL_RPATH_USE_LINK_PATH ON
)

target_link_libraries(NotificationServiceTests
    PRIVATE
        NotificationServiceCore
        Catch2::Catch2
        Qt6::Widgets
        Qt6::Test
)

set_target_properties(NotificationServiceTests PROPERTIES AUTOMOC ON)

include(CTest)
enable_testing()
include(Catch)
catch_discover_tests(NotificationServiceTests)
