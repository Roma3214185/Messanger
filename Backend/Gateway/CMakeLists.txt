cmake_minimum_required(VERSION 3.16)
project(Gateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GATEWAY_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(GATEWAY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(nlohmann_json REQUIRED)
find_package(fmt REQUIRED)

include(FetchContent)

FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.3
)
FetchContent_MakeAvailable(Crow)

add_library(GatewayCore
    ${GATEWAY_SRC_DIR}/gatewayserver.cpp
    ${GATEWAY_SRC_DIR}/proxyclient.cpp
    ${GATEWAY_SRC_DIR}/JWTVerifier.cpp
    ${GATEWAY_SRC_DIR}/websocketbridge.cpp
)

target_include_directories(GatewayCore PUBLIC
    ${GATEWAY_INCLUDE_DIR}/
    ${httplib_SOURCE_DIR}
    ${BOOST_ROOT}/include
    /opt/homebrew/include
)

target_link_libraries(GatewayCore PUBLIC
    ${CMAKE_THREAD_LIBS_INIT}
    Crow::Crow
    prometheus-cpp::core
    prometheus-cpp::pull
    ixwebsocket
    jwt-cpp
    Metrics
    RedisCache
    nlohmann_json::nlohmann_json
    Constants
    RabbitMQClient
    ThreadPool
    fmt::fmt
    BackendMocks
)

add_executable(Gateway
    ${GATEWAY_SRC_DIR}/main.cpp
)

target_link_libraries(Gateway PRIVATE GatewayCore)

set_target_properties(Gateway PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "@executable_path;@executable_path/../lib;/opt/homebrew/lib;/usr/local/lib;/opt/homebrew/opt/redis-plus-plus/lib"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
