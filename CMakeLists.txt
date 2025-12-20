cmake_minimum_required(VERSION 3.22)

# if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
#     set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake"
#         CACHE FILEPATH "vcpkg toolchain file")
# endif()

project(Chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CROW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CROW_BUILD_CGI OFF CACHE BOOL "" FORCE)
set(CROW_DISABLE_SSL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(RABBITMQ_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)

set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(WITH_BENCHMARK OFF CACHE BOOL "" FORCE)
set(WITH_EXAMPLES OFF CACHE BOOL "" FORCE)
set(WITH_TEST OFF CACHE BOOL "" FORCE)
set(WITH_OTLP_GRPC ON CACHE BOOL "" FORCE)
set(WITH_OTLP_HTTP OFF CACHE BOOL "" FORCE)
set(gRPC_SSL_PROVIDER "package" CACHE STRING "" FORCE)
set(WITH_OTLP OFF)
set(WITH_OTLP_GRPC OFF)
set(WITH_OTLP_HTTP OFF)
set(WITH_PROMETHEUS OFF)
set(WITH_ZIPKIN OFF)
set(BUILD_TESTING OFF)
set(WITH_EXAMPLES OFF)

set(CMAKE_PREFIX_PATH "/opt/homebrew" ${CMAKE_PREFIX_PATH})
add_definitions(-DQT_NO_KEYWORDS -DCROW_USE_ASIO)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

# -------------------------
# Global INTERFACE target for warnings, sanitizers, PCH
# -------------------------
add_library(ProjectOptions INTERFACE)

# Include compiler warnings
# include(cmake/CompilerWarnings.cmake)
# set_project_warnings(ProjectOptions)

# Include sanitizers if supported
include(cmake/Sanitizers.cmake)
enable_sanitizers(ProjectOptions)

# Precompiled headers (optional)
# option(ENABLE_PCH "Enable Precompiled Headers" OFF)
# if (ENABLE_PCH)
#     target_precompile_headers(ProjectOptions INTERFACE
#         <vector>
#         <string>
#         <map>
#         <utility>)
# endif()

# Static analyzers
include(cmake/StaticAnalyzers.cmake)

# -------------------------
# Submodules
# -------------------------
add_subdirectory(external/IXWebSocket)
add_subdirectory(external/jwt-cpp)
add_subdirectory(external/tracy)
add_subdirectory(external/SimpleAmqpClient)
add_subdirectory(external/prometheus-cpp)

# -------------------------
# Common Libraries
# -------------------------
add_subdirectory(common/Entities build_entities)
add_subdirectory(common/Metrics build_metrics)
add_subdirectory(common/RedisCache build_redis)
add_subdirectory(common/RabbitMQClient build_rabbit)
#add_subdirectory(common/Persistence build_persistence)
add_subdirectory(common/Network build_network)
add_subdirectory(common/Constants build_constants)
add_subdirectory(common/ThreadPool build_thread_pool)

# -------------------------
# Microservices
# -------------------------
set(ALL_SERVICES
    AuthService
    MessageService
    ChatService
    Gateway
    NotificationService
)

if (DEFINED TARGET)
    message(STATUS "Building only microservice: ${TARGET}")

    list(FIND ALL_SERVICES ${TARGET} _index)
    if (_index EQUAL -1)
        message(FATAL_ERROR "Unknown TARGET '${TARGET}'. Valid: ${ALL_SERVICES}")
    endif()

    add_subdirectory(Backend/${TARGET})
    target_link_libraries(${TARGET} INTERFACE ProjectOptions)

else()
    message(STATUS "Building ALL targets")

    foreach(service ${ALL_SERVICES})
        add_subdirectory(Backend/${service})
        #target_link_libraries(${service} INTERFACE ProjectOptions)
    endforeach()

    # Frontend
    add_subdirectory(Frontend)
    target_link_libraries(Frontend INTERFACE ProjectOptions)

    # Mocks for backend tests
    add_subdirectory(Backend/common_mocks)

    # Tests
    include(CTest)
    enable_testing()

    add_subdirectory(common/Persistence/tests)
    add_subdirectory(Frontend/tests)
    foreach(service ${ALL_SERVICES})
        add_subdirectory(Backend/${service}/tests)
    endforeach()
endif()

# Benchmarks
# add_subdirectory(Backend/Gateway/benchmarks)
# add_subdirectory(Backend/AuthService/benchmarks)

