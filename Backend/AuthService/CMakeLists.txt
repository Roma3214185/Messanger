cmake_minimum_required(VERSION 3.16)

project(AuthService LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)

add_executable(AuthService
  main.cpp

  src/AuthController/authcontroller.h
  src/AuthController/authcontroller.cpp

  src/AuthManager/authmanager.cpp
  src/AuthManager/authmanager.h

  src/Headers/RegisterRequest.h
  src/Headers/AuthResponce.h
  src/Headers/User.h
  src/Headers/JwtUtils.h

  src/DataBase/databace.cpp
  src/DataBase/database.h

  src/Server/server.h
  src/Server/server.cpp

)

target_link_libraries(AuthService PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Sql
  Crow::Crow
  SQLite::SQLite3
)

target_include_directories(AuthService PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(GNUInstallDirs)
install(TARGETS AuthService
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
