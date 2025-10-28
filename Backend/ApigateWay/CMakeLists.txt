cmake_minimum_required(VERSION 3.16)
project(ApiGateWay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.11.2
)
FetchContent_MakeAvailable(httplib)

add_executable(ApiGateWay
    main.cpp
    src/GateWayServer/gatewayserver.cpp
    src/ProxyClient/proxyclient.cpp
)

target_include_directories(ApiGateWay PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GateWayServer
    ${CMAKE_CURRENT_SOURCE_DIR}/external/json/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/crow/include
    ${httplib_SOURCE_DIR}
)

target_link_libraries(ApiGateWay PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
)
