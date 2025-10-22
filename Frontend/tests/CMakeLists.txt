cmake_minimum_required(VERSION 3.16)
project(FrontendServiceTests)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

# Catch2
find_package(Catch2 REQUIRED)

# Qt (для QString, QObject, QNetworkRequest тощо)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network WebSockets Gui Test)
find_package(spdlog REQUIRED)

# Бібліотека з твоїм кодом для тестів (без реального Model.cpp)
add_library(FrontendServiceLib
    ../src/Presenter/presenter.cpp
    ../src/MessageModel/messagemodel.cpp
    ../src/MessageDelegate/messagedelegate.cpp
    ../src/ChatModel/chatmodel.cpp
    ../src/headers/MockCache.h
    ../src/NetworkAccessManager/MockAccessManager.h
    ../src/Model/model.cpp
    ../src/UserModel/UserModel.cpp
    ../src/headers/MockReply.h
)

# Інклуди
target_include_directories(FrontendServiceLib
    PUBLIC
        ../src
        ${Qt6Core_INCLUDE_DIRS}
        ${Qt6Widgets_INCLUDE_DIRS}
        ${Qt6Gui_INCLUDE_DIRS}
        ${Qt6Network_INCLUDE_DIRS}
        ${Qt6WebSockets_INCLUDE_DIRS}
)

# Лінкування Qt
target_link_libraries(FrontendServiceLib
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::Network
        Qt6::WebSockets
        Qt6::Test
    spdlog::spdlog
    #hiredis
)

# Тестовий екзешник
add_executable(FrontendServiceTests
    presenter_tests.cpp
        model_tests.cpp
)

# Лінкування тестів з бібліотекою та Catch2
target_link_libraries(FrontendServiceTests
    PRIVATE
        FrontendServiceLib
        Catch2::Catch2WithMain
)

# Реєстрація тестів
include(CTest)
include(Catch)
catch_discover_tests(FrontendServiceTests)
