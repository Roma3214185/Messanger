cmake_minimum_required(VERSION 3.16)
project(NotificationServiceTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network WebSockets Gui Test)
find_package(Catch2 3 REQUIRED)

add_executable(NotificationServiceTests
    test_rabitmqpublisher.cpp
    main.cpp
    test_socket_repository.cpp
    test_socket_notifier.cpp
    test_rabbitsubscriber.cpp

    mocks/notificationservice/src/MockUserSocketRepository.cpp
    mocks/notificationservice/src/MockNotifier.cpp
    mocks/notificationservice/src/MockSocket.cpp
    test_socketHandlersRepository.cpp
    test_messageHandlers.cpp

)

set_target_properties(NotificationServiceTests PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
)

target_link_libraries(NotificationServiceTests
    PRIVATE
        NotificationServiceCore
        Catch2::Catch2
        Qt6::Widgets
        Qt6::Test
        ProjectOptions
        BackendMocks
)

target_include_directories(NotificationServiceTests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(NotificationServiceTests PROPERTIES AUTOMOC ON)

include(CTest)
enable_testing()
include(Catch)
catch_discover_tests(NotificationServiceTests)
