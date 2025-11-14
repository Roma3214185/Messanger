cmake_minimum_required(VERSION 3.16)
project(ApiGateWay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(APIGATE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(APIGATE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BOOST_ROOT "/opt/homebrew/opt/boost@1.89")

find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)

include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.11.2
)
FetchContent_MakeAvailable(httplib)

# add_subdirectory(
#     ${CMAKE_CURRENT_SOURCE_DIR}/../../external/IXWebSocket
#     ${CMAKE_CURRENT_BINARY_DIR}/external/IXWebSocket-build
# )

add_executable(ApiGateWay
    ${APIGATE_SRC_DIR}/main.cpp
    ${APIGATE_SRC_DIR}/gatewayserver.cpp
    ${APIGATE_SRC_DIR}/proxyclient.cpp
    ${APIGATE_SRC_DIR}/JwtUtils.cpp
    ${APIGATE_SRC_DIR}/websocketbridge.cpp
)

target_include_directories(ApiGateWay PRIVATE
    ${APIGATE_INCLUDE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    #${CMAKE_CURRENT_SOURCE_DIR}/../external
    ${httplib_SOURCE_DIR}
    ${BOOST_ROOT}/include
    /opt/homebrew/include
)

target_link_libraries(ApiGateWay PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
    Crow::Crow
    ixwebsocket
    jwt-cpp
    Metrics
    nlohmann_json::nlohmann_json
    Constants
)

set_target_properties(ApiGateWay PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "@executable_path;@executable_path/../lib;/opt/homebrew/lib;/usr/local/lib;/opt/homebrew/opt/redis-plus-plus/lib"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
