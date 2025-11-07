cmake_minimum_required(VERSION 3.16)

project(AuthService LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(AUTH_SERVICE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(AUTH_SERVICE_INCUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)
find_package(spdlog REQUIRED)

add_executable(AuthService
  ${AUTH_SERVICE_SRC_DIR}/main.cpp
  ${AUTH_SERVICE_SRC_DIR}/authcontroller.cpp
  ${AUTH_SERVICE_SRC_DIR}/authmanager.cpp
  ${AUTH_SERVICE_SRC_DIR}/JwtUtils.cpp
  ${AUTH_SERVICE_SRC_DIR}/server.cpp
)


target_link_libraries(AuthService PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Sql
  Crow::Crow
  SQLite::SQLite3
  spdlog::spdlog
  redis++
  TracyClient
  jwt-cpp
  #RedisCache
)

target_include_directories(AuthService PRIVATE
  ${AUTH_SERVICE_INCUDE_DIR}/
)

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
