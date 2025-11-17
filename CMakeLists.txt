cmake_minimum_required(VERSION 3.22)
project(Chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(RABBITMQ_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)

set(CMAKE_PREFIX_PATH "/opt/homebrew" ${CMAKE_PREFIX_PATH})

add_definitions(-DQT_NO_KEYWORDS -DCROW_USE_ASIO)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

# -------------------------
# Submodules
# -------------------------
add_subdirectory(external/crow)
add_subdirectory(external/IXWebSocket)
add_subdirectory(external/jwt-cpp)
add_subdirectory(external/tracy)
add_subdirectory(external/SimpleAmqpClient)
add_subdirectory(external/prometheus-cpp)

# -------------------------
# My libraries
# -------------------------
add_subdirectory(common/Entities build_entities)
add_subdirectory(common/Metrics build_metrics)
add_subdirectory(common/RedisCache build_redis)
add_subdirectory(common/RabbitMQClient build_rabbit)
add_subdirectory(common/Persistence build_persistence)
add_subdirectory(common/NetworkManager build_networkmanager)
add_subdirectory(common/constants build_constants)

# -------------------------
# Microservices
# -------------------------
add_subdirectory(Backend/AuthService)
add_subdirectory(Backend/MessageService)
add_subdirectory(Backend/ChatService)
add_subdirectory(Backend/ApiGateway)
add_subdirectory(Backend/NotificationService)

# -------------------------
# Frontend
# -------------------------
add_subdirectory(Frontend)


# -------------------------
# Mocks for backend tests
# -------------------------
add_subdirectory(Backend/common_mocks)

# -------------------------
# Tests
# -------------------------
include(CTest)
enable_testing()
add_subdirectory(common/Persistence/tests)
add_subdirectory(Frontend/tests)
add_subdirectory(Backend/NotificationService/tests)
add_subdirectory(Backend/MessageService/tests)
add_subdirectory(Backend/ChatService/tests)
