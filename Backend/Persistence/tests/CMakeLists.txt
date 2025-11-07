cmake_minimum_required(VERSION 3.16)
project(PersistenceTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network WebSockets Gui Test)
find_package(spdlog REQUIRED)
find_package(Catch2 3 REQUIRED)

set(PERSISTENCE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)
set(PERSISTENCE_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Persistence/include)
set(DEBUG_PROFILING_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../DebugProfiling)

add_library(PersistenceLib_local
    ${PERSISTENCE_SRC_DIR}/GenericRepository.cpp

    mocks/MockDatabase.h
)

target_include_directories(PersistenceLib_local PUBLIC
    ${PERSISTENCE_INC_DIR}
    /usr/local/include
    /opt/homebrew/include
)

target_link_libraries(PersistenceLib_local PUBLIC
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
    Qt6::Gui
    spdlog::spdlog
)

target_compile_definitions(PersistenceLib_local PUBLIC SPDLOG_HEADER_ONLY)

add_executable(PersistenceTests
    main.cpp
    test_genericrepository.cpp
)

set_target_properties(PersistenceTests PROPERTIES
    AUTOMOC ON
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "/opt/homebrew/lib"
    INSTALL_RPATH_USE_LINK_PATH ON
)

target_link_libraries(PersistenceTests
    PRIVATE
        PersistenceLib_local
        Catch2::Catch2
        Qt6::Widgets
        Qt6::Test
)

include(CTest)
include(Catch)

# catch_discover_tests(PersistenceTests)
