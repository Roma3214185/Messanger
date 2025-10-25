cmake_minimum_required(VERSION 3.16)

project(NotificationService LANGUAGES CXX)

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
find_package(spdlog REQUIRED)

link_directories(/opt/homebrew/lib)


add_executable(NotificationService
  main.cpp
  Headers/Message.h
  NotificationManager/notificationmanager.h NotificationManager/notificationmanager.cpp
  NetworkManager/networkmanager.h NetworkManager/networkmanager.cpp

)

target_link_libraries(NotificationService PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::WebSockets
  Qt${QT_VERSION_MAJOR}::Sql
  Crow::Crow
  redis++
  spdlog::spdlog
  #TracyClient
  #SimpleAmqpClient
  #rabbitmq
  #boost_system
  #boost_chrono
  jwt-cpp
)

include_directories(
    ${httplib_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/MessageDataBase
    ${CMAKE_CURRENT_SOURCE_DIR}/Controller
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers
)


include(GNUInstallDirs)
install(TARGETS NotificationService
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set_target_properties(NotificationService PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "@executable_path;@executable_path/../lib;/opt/homebrew/lib;/usr/local/lib"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
