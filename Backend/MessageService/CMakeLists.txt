cmake_minimum_required(VERSION 3.16)

project(MessageService LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.11.2
)

FetchContent_MakeAvailable(httplib)

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets Sql)
find_package(Crow REQUIRED)


add_executable(MessageService
  main.cpp
  MessageDataBase/messagedatabase.cpp
  Controller/controller.cpp
  Server/server.h Server/server.cpp
  Headers/Message.h
)

target_link_libraries(MessageService PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::WebSockets
  Qt${QT_VERSION_MAJOR}::Sql
  Crow::Crow
)

include_directories(
    ${httplib_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/MessageDataBase
    ${CMAKE_CURRENT_SOURCE_DIR}/Controller
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers
)


include(GNUInstallDirs)
install(TARGETS MessageService
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
