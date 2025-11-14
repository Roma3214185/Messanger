cmake_minimum_required(VERSION 3.16)

project(MessageService LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MESSAGE_SERVICE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MESSAGE_SERVICE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.11.2
)

FetchContent_MakeAvailable(httplib)

find_package(Qt6 REQUIRED COMPONENTS Core WebSockets Sql)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

link_directories(/opt/homebrew/lib)


add_executable(MessageService
  ${MESSAGE_SERVICE_SRC_DIR}/main.cpp

  ${MESSAGE_SERVICE_SRC_DIR}/controller.cpp
  ${MESSAGE_SERVICE_SRC_DIR}/server.cpp
  ${MESSAGE_SERVICE_SRC_DIR}/messageworker.cpp
  ${MESSAGE_SERVICE_SRC_DIR}/managers/MessageManager.cpp
  ${MESSAGE_SERVICE_SRC_DIR}/JwtUtils.cpp
)

target_link_libraries(MessageService PRIVATE
  Crow::Crow
  Entities
  RabbitMQClient
  jwt-cpp
  nlohmann_json::nlohmann_json
  Constants
  NetworkManager
)

include_directories(
    ${httplib_SOURCE_DIR}
    ${MESSAGE_SERVICE_INCLUDE_DIR}/
)


include(GNUInstallDirs)
install(TARGETS MessageService
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set_target_properties(MessageService PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "@executable_path;@executable_path/../lib;/opt/homebrew/lib;/usr/local/lib"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
