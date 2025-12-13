cmake_minimum_required(VERSION 3.22)
project(Network LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(nlohmann_json REQUIRED)

include(FetchContent)

# Fetch cpp-httplib
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.11.2
)
FetchContent_MakeAvailable(httplib)

# Create Network library
add_library(Network STATIC
    src/IChatNetworkManager.cpp
    src/INetworkManagerBase.cpp
    src/IUserNetworkManager.cpp
    src/IMessageNetworkManager.cpp
)

# Include directories
target_include_directories(Network
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        SYSTEM          # SYSTEM tells compiler these are external headers
        ${httplib_SOURCE_DIR}/
)

target_compile_options(Network PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-old-style-cast>
)

find_program(CLANG_TIDY_EXECUTABLE /opt/homebrew/opt/llvm/bin/clang-tidy)
if(CLANG_TIDY_EXECUTABLE)
    set_target_properties(Network PROPERTIES
        CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE};-header-filter=^${CMAKE_CURRENT_SOURCE_DIR}/include"
    )
endif()

# Link libraries
target_link_libraries(Network PRIVATE
    Entities
    Metrics
    Crow::Crow
    Constants
)
