cmake_minimum_required(VERSION 3.16)
project(PersistenceTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
add_definitions(-DQT_NO_KEYWORDS -DCROW_USE_ASIO)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)
find_package(spdlog REQUIRED)
find_package(Catch2 3 REQUIRED)

#add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/PersistenceBuild)

add_executable(PersistenceTests
    test_genericrepository.cpp
    test_query.cpp
    test_sqlitedatabase.cpp
    main.cpp
)

target_link_libraries(PersistenceTests
    PUBLIC
        Persistence
        Metrics
        Entities
        Qt6::Core
        Qt6::Sql
        Catch2::Catch2WithMain
        spdlog::spdlog
)

target_include_directories(PersistenceTests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/../include
        ${CMAKE_SOURCE_DIR}/../../Metrics/include
        ${CMAKE_SOURCE_DIR}/../inl
        ${CMAKE_SOURCE_DIR}/../../RedisCache/include
        ${CMAKE_SOURCE_DIR}/../../entities/include
)

include(CTest)
enable_testing()
include(Catch)
catch_discover_tests(PersistenceTests)
