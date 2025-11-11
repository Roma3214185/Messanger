cmake_minimum_required(VERSION 3.16)
project(FrontendServiceTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network WebSockets Gui Test)
find_package(spdlog REQUIRED)
find_package(Catch2 3 REQUIRED)

add_executable(FrontendServiceTests
    main.cpp
    test_datainputservice.cpp
    test_presenter.cpp
    test_model.cpp
    test_sessionmanager.cpp
    test_socketmanager.cpp
    test_messagemanager.cpp
    test_usermanager.cpp
    test_chatmanager.cpp
)

set_target_properties(FrontendServiceTests PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "/opt/homebrew/lib"
    INSTALL_RPATH_USE_LINK_PATH ON
)

target_link_libraries(FrontendServiceTests
    PRIVATE
        FrontendLib
        Metrics
        Catch2::Catch2
        Qt6::Widgets
        Qt6::Test
        prometheus-cpp::core
        prometheus-cpp::pull
)

set_target_properties(FrontendServiceTests PROPERTIES AUTOMOC ON)

include(CTest)
enable_testing()
include(Catch)
catch_discover_tests(FrontendServiceTests)
