cmake_minimum_required(VERSION 3.16)

project(AuthService LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(AUTH_SERVICE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(AUTH_SERVICE_INCUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

link_directories(
    /usr/local/lib
    /opt/homebrew/lib
)

include(FetchContent)
FetchContent_Declare(
    libsodium
    GIT_REPOSITORY https://github.com/jedisct1/libsodium.git
    GIT_TAG        stable
)
FetchContent_MakeAvailable(libsodium)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)
find_package(SQLite3 REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(AuthServiceCore STATIC
  ${AUTH_SERVICE_SRC_DIR}/authcontroller.cpp
  ${AUTH_SERVICE_SRC_DIR}/authmanager.cpp
  ${AUTH_SERVICE_SRC_DIR}/JwtUtils.cpp
  ${AUTH_SERVICE_SRC_DIR}/server.cpp
)

target_link_libraries(AuthServiceCore PUBLIC
  Qt${QT_VERSION_MAJOR}::Core
  Crow::Crow
  SQLite::SQLite3
  jwt-cpp
  sodium
  Entities
  Constants
  NetworkManager
  nlohmann_json::nlohmann_json
)

target_include_directories(AuthServiceCore PUBLIC
  ${AUTH_SERVICE_INCUDE_DIR}/
  ${libsodium_SOURCE_DIR}/src/libsodium/include
)

add_executable(AuthService
  ${AUTH_SERVICE_SRC_DIR}/main.cpp
)

target_link_libraries(AuthService PRIVATE AuthServiceCore)

include(GNUInstallDirs)
install(TARGETS AuthService
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set_target_properties(AuthService PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "@executable_path;@executable_path/../lib;/opt/homebrew/lib;/usr/local/lib"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
