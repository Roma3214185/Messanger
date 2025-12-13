cmake_minimum_required(VERSION 3.22)
project(Entities LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT TARGET Entities)

add_library(Entities INTERFACE)

target_include_directories(Entities INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Entities INTERFACE ProjectOptions)

target_link_libraries(Entities INTERFACE
    nlohmann_json::nlohmann_json
    Crow::Crow
    Metrics
)

endif()

