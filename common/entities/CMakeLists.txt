cmake_minimum_required(VERSION 3.22)
project(Entities LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS core)
find_package(nlohmann_json REQUIRED)

add_library(Entities INTERFACE)

target_include_directories(Entities INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/json/include
)

target_link_libraries(Entities INTERFACE
    Qt6::Core Persistence
    nlohmann_json
    Crow::Crow
)

