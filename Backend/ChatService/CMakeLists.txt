cmake_minimum_required(VERSION 3.10)
project(ChatService LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)
find_package(spdlog REQUIRED)

include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.11.2
)
FetchContent_MakeAvailable(httplib)

# Додаємо шляхи до своїх заголовків
include_directories(
    ${httplib_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DataBase
    ${CMAKE_CURRENT_SOURCE_DIR}/src/headers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Controller
)

add_executable(ChatService
    main.cpp
    src/DataBase/database.cpp
    src/Controller/controller.cpp
    src/Server/server.h src/Server/server.cpp
    src/headers/Chat.h
    src/headers/NetworkManager.cpp
)

target_link_libraries(ChatService PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
    Crow::Crow
    jwt-cpp
    hiredis
    spdlog::spdlog
    TracyClient
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
