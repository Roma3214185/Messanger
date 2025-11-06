cmake_minimum_required(VERSION 3.10)
project(ChatService LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)
find_package(spdlog REQUIRED)

set(CHAT_SERVICE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CHAT_SERVICE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.11.2
)
FetchContent_MakeAvailable(httplib)

add_executable(ChatService
    ${CHAT_SERVICE_SRC_DIR}/main.cpp
    ${CHAT_SERVICE_SRC_DIR}/database.cpp
    ${CHAT_SERVICE_SRC_DIR}/controller.cpp
    ${CHAT_SERVICE_SRC_DIR}/server.cpp
    ${CHAT_SERVICE_SRC_DIR}/NetworkManager.cpp
    ${CHAT_SERVICE_SRC_DIR}/chatmanager.cpp
)

target_include_directories(ChatService PRIVATE
  ${httplib_SOURCE_DIR}
  ${CHAT_SERVICE_INCLUDE_DIR}/
)

target_link_libraries(ChatService PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
    Crow::Crow
    jwt-cpp
    hiredis
    spdlog::spdlog
    TracyClient
    RabbitMQClient
    RedisCache
    GenericRepository
)

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
