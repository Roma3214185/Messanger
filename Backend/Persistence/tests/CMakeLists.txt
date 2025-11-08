cmake_minimum_required(VERSION 3.16)
project(PersistenceTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Test)
find_package(spdlog REQUIRED)
find_package(Catch2 3 REQUIRED)

target_compile_definitions(Persistence PUBLIC SPDLOG_HEADER_ONLY)

add_executable(PersistenceTests
    main.cpp
    test_genericrepository.cpp
    test_query.cpp
    mocks/MockQuery.h
)

target_include_directories(Persistence
    PUBLIC include
)

set_target_properties(PersistenceTests PROPERTIES
    AUTOMOC ON
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "/opt/homebrew/lib"
    INSTALL_RPATH_USE_LINK_PATH ON
)

target_link_libraries(PersistenceTests
    PRIVATE
        Persistence
        Catch2::Catch2
        Qt6::Test
        spdlog::spdlog
)

include(CTest)
include(Catch)

# catch_discover_tests(PersistenceTests)
