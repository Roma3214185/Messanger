cmake_minimum_required(VERSION 3.22)
project(Persistence LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)
find_package(nlohmann_json REQUIRED)

if (NOT TARGET Persistence)

add_library(Persistence STATIC
    src/GenericRepository.cpp
    src/SQLiteDatabase.cpp
    src/SqlExecutor.cpp
)

target_include_directories(Persistence
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/inl
)

target_link_libraries(Persistence
    PUBLIC
        Qt6::Core
        Qt6::Sql
        Metrics
        ThreadPool
        RedisCache
        Entities
        nlohmann_json::nlohmann_json
)

endif()
