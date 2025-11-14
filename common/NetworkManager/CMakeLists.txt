cmake_minimum_required(VERSION 3.22)
project(NetworkManager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS core)
find_package(nlohmann_json REQUIRED)

include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.11.2
)
FetchContent_MakeAvailable(httplib)

add_library(NetworkManager STATIC
    src/IChatNetworkManager.cpp
    src/INetworkManager.cpp
    src/IUserNetworkManager.cpp
    src/IMessageNetworkManager.cpp
    include/interfaces/IMessageNetworkManager.h
)

target_include_directories(NetworkManager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${httplib_SOURCE_DIR}
)

target_link_libraries(NetworkManager
    Entities
    Metrics
    Crow::Crow
    Constants
)
