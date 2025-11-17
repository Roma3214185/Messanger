cmake_minimum_required(VERSION 3.10)
project(ChatService LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

set(CHAT_SERVICE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CHAT_SERVICE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
find_package(nlohmann_json REQUIRED)

add_library(ChatServiceCore STATIC
  ${CHAT_SERVICE_SRC_DIR}/chatcontroller.cpp
  ${CHAT_SERVICE_SRC_DIR}/chatserver.cpp
  ${CHAT_SERVICE_SRC_DIR}/chatmanager.cpp
  ${CHAT_SERVICE_SRC_DIR}/TokenService.cpp
)

target_link_libraries(ChatServiceCore PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
    Crow::Crow
    jwt-cpp
    nlohmann_json::nlohmann_json
    RabbitMQClient
    Entities
    NetworkManager
)

target_include_directories(ChatServiceCore PUBLIC
  ${httplib_SOURCE_DIR}
  ${CHAT_SERVICE_INCLUDE_DIR}/
)

add_executable(ChatService
    ${CHAT_SERVICE_SRC_DIR}/main.cpp
)

target_link_libraries(ChatService PRIVATE ChatServiceCore Qt6::Core)

include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

include(GNUInstallDirs)
install(TARGETS ChatService
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set_target_properties(ChatService PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "@executable_path;@executable_path/../lib;/opt/homebrew/lib;/usr/local/lib"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
