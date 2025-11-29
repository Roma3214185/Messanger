cmake_minimum_required(VERSION 3.16)
project(AuthService LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(QT_NO_KEYWORDS)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../common/Persistence ../../build_persistence)

include(FetchContent)

FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.3
)
FetchContent_MakeAvailable(Crow)

find_package(Qt6 REQUIRED COMPONENTS Core Sql Widgets Gui)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

link_directories(/opt/homebrew/lib /usr/local/lib)

add_library(AuthServiceCore STATIC
    src/authcontroller.cpp
    src/authmanager.cpp
    src/JwtUtils.cpp
    src/server.cpp
    src/authDataInputService.cpp
)

target_include_directories(AuthServiceCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SODIUM_INCLUDE_DIRS}
)

target_compile_options(AuthServiceCore PRIVATE -Wno-deprecated-declarations)

target_link_libraries(AuthServiceCore PUBLIC
    Qt6::Core Qt6::Sql Qt6::Widgets Qt6::Gui
    OpenSSL::SSL OpenSSL::Crypto
    ${SODIUM_LIBRARIES}
    Crow::Crow
    jwt-cpp
    Metrics
    sodium
    Constants
    Entities
    Network
    Persistence
    nlohmann_json::nlohmann_json
)

add_executable(AuthService src/main.cpp)
target_link_libraries(AuthService PRIVATE AuthServiceCore)
