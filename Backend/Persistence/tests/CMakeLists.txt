cmake_minimum_required(VERSION 3.16)
project(PersistenceTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_definitions(-DQT_NO_KEYWORDS -DCROW_USE_ASIO)

find_package(Qt6 REQUIRED COMPONENTS Gui Test Sql Core)
find_package(spdlog REQUIRED)
find_package(Catch2 3 REQUIRED)

add_library(Persistence_local
    ../../Persistence/src/GenericRepository.cpp
    ../../Persistence/src/SQLiteDatabase.cpp
    ../../Persistence/src/ThreadPool.cpp
    ../../Persistence/src/SqlExecutor.cpp

)

target_include_directories(Persistence_local PUBLIC
    ../../Persistence/include
    ../../../DebugProfiling
    ../../../Backend
)

target_link_libraries(Persistence_local
    PUBLIC
        Qt6::Core
        Qt6::Sql
        Catch2::Catch2
        Qt6::Test
        spdlog::spdlog
        Qt6::Sql
)

add_executable(PersistenceTests
    main.cpp
    test_genericrepository.cpp
    test_query.cpp
    mocks/MockQuery.h
)

target_link_libraries(PersistenceTests
    PRIVATE
        Persistence_local
)

include(CTest)
include(Catch)

# catch_discover_tests(PersistenceTests)
