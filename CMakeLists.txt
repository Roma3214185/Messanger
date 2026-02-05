cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE FILEPATH "vcpkg toolchain file")

set(VCPKG_TARGET_TRIPLET arm64-osx)

set(QT_CREATOR_SKIP_VCPKG_SETUP ON CACHE BOOL "Skip Qt Creator vcpkg auto-setup")

set(VCPKG_INSTALLED_DIR
    "${CMAKE_SOURCE_DIR}/vcpkg_installed"
    CACHE PATH "vcpkg install directory")

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
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

add_definitions(-DQT_NO_KEYWORDS -DCROW_USE_ASIO)

set(Protobuf_ROOT
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}"
    CACHE PATH "")

# -------------------------
# Global INTERFACE target for warnings, sanitizers, PCH
# -------------------------
add_library(ProjectOptions INTERFACE)

# Precompiled headers (optional)
# option(ENABLE_PCH "Enable Precompiled Headers" OFF)
# if (ENABLE_PCH)
#     target_precompile_headers(ProjectOptions INTERFACE
#         <vector>
#         <string>
#         <map>
#         <utility>)
# endif()

option(ENABLE_WARNINGS "Enable compiler warnings" OFF)
option(ENABLE_CLANG_FORMAT "Enable clang format" OFF)
option(ENABLE_CLANG_TIDY "Enable clang tidy" OFF)
option(ENABLE_CPPCHECK "Enable cpp check" OFF)
option(ENABLE_IWYU "Enable include what you use" ON)
set(ENABLE_SANITIZER_ADDRESS ON CACHE BOOL "" FORCE)
set(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR ON CACHE BOOL "" FORCE)

# if (ENABLE_WARNINGS)
#     include(cmake/CompilerWarnings.cmake)
#     #set_project_warnings(ProjectOptions)
# endif()

include(cmake/CompilerWarnings.cmake)

option(ENABLE_COVERAGE "Enable coverage flags" OFF)
if(ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    enable_coverage(ProjectOptions)
endif()

target_include_directories(ProjectOptions
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/common/include
    SYSTEM
        ${CMAKE_SOURCE_DIR}/external
        ${CMAKE_SOURCE_DIR}/build
        ${CMAKE_BINARY_DIR}/_deps
)

# -------------------------
# Static analyzers
# -------------------------
include(cmake/StaticAnalyzers.cmake)
include(cmake/ClangFormat.cmake)

# -------------------------
# Sanitizers
# -------------------------
set(CLANG_TIDY_EXE "/opt/homebrew/opt/llvm/bin/clang-tidy")
include(cmake/Sanitizers.cmake)

# -------------------------
# Submodules
# -------------------------
add_subdirectory(external)

# -------------------------
# Common Libraries
# -------------------------
macro(add_project_common name)
    add_subdirectory(common/${name} build_${name})
    target_link_libraries(${name} INTERFACE ProjectOptions)
endmacro()

add_project_common(Entities)
add_project_common(Metrics)
add_project_common(RedisCache)
add_project_common(RabbitMQClient)
add_project_common(Network)
add_project_common(Constants)
add_project_common(ThreadPool)

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
    target_link_libraries(${TARGET} PRIVATE ProjectOptions)

else()
    message(STATUS "Building ALL targets")

    foreach(service ${ALL_SERVICES})
        add_subdirectory(Backend/${service})
        target_link_libraries(${service} PRIVATE ProjectOptions)
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

