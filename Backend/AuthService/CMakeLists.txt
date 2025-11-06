cmake_minimum_required(VERSION 3.16)

project(AuthService LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(FRONTEND_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FRONTEND_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)
find_package(spdlog REQUIRED)

add_executable(AuthService
  ${FRONTEND_SRC_DIR}/main.cpp
  ${FRONTEND_SRC_DIR}/authcontroller.cpp
  ${FRONTEND_SRC_DIR}/authmanager.cpp
  ${FRONTEND_SRC_DIR}/JwtUtils.cpp
  ${FRONTEND_SRC_DIR}/server.cpp
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
  RedisCache
)

target_include_directories(AuthService PRIVATE
  ${FRONTEND_INC_DIR}/
  # ${CMAKE_CURRENT_SOURCE_DIR}/src
  # ${CMAKE_CURRENT_SOURCE_DIR}/src/Headers
  # ${CMAKE_CURRENT_SOURCE_DIR}/src/Server
  # ${CMAKE_CURRENT_SOURCE_DIR}/src/AuthManager
  # ${CMAKE_CURRENT_SOURCE_DIR}/src/AuthController
  # ${CMAKE_CURRENT_SOURCE_DIR}/../RedisCache
  # ${CMAKE_CURRENT_SOURCE_DIR}/../GenericRepository
  # ${CMAKE_CURRENT_SOURCE_DIR}/../../DebugProfiling
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
