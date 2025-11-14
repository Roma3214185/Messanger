cmake_minimum_required(VERSION 3.16)
project(NotificationService LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(NOTIFICATION_SERVICE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(NOTIFICATION_SERVICE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(FetchContent)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.11.2
)
FetchContent_MakeAvailable(httplib)

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets Sql Test)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Catch2 3 REQUIRED)

add_library(NotificationServiceCore STATIC
    ${NOTIFICATION_SERVICE_SRC_DIR}/managers/notificationmanager.cpp
    ${NOTIFICATION_SERVICE_SRC_DIR}/managers/socketmanager.cpp
    ${NOTIFICATION_SERVICE_SRC_DIR}/server.cpp
)

target_include_directories(NotificationServiceCore PUBLIC
    ${NOTIFICATION_SERVICE_INCLUDE_DIR}
    ${httplib_SOURCE_DIR}
    /opt/homebrew/include
)

target_link_directories(NotificationServiceCore PRIVATE
    /opt/homebrew/lib
)

target_link_libraries(NotificationServiceCore PUBLIC
    Qt6::Core
    Qt6::WebSockets
    Qt6::Sql
    Crow::Crow
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    Metrics
    jwt-cpp
    RabbitMQClient
    NetworkManager
    Constants
)

add_executable(NotificationService
    ${NOTIFICATION_SERVICE_SRC_DIR}/main.cpp
)

target_link_libraries(NotificationService PRIVATE NotificationServiceCore Qt6::Core)

set_target_properties(NotificationService PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "@executable_path;@executable_path/../lib;/opt/homebrew/lib;/usr/local/lib"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

include(GNUInstallDirs)
install(TARGETS NotificationService
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
