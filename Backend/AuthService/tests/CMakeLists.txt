cmake_minimum_required(VERSION 3.16)
project(AuthServiceTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network WebSockets Gui Test)
find_package(spdlog REQUIRED)
find_package(Catch2 3 REQUIRED)

link_directories(
    /usr/local/lib
    /opt/homebrew/lib
)

include(FetchContent)
FetchContent_Declare(
    libsodium
    GIT_REPOSITORY https://github.com/jedisct1/libsodium.git
    GIT_TAG        stable
)
FetchContent_MakeAvailable(libsodium)

add_executable(AuthServiceTests
    main.cpp
    test_authserver.cpp
    mocks/MockAuthManager.h
    mocks/MockGenerator.h
    test_authmanager.cpp
    test_protected_authmanager.cpp
)

set_target_properties(AuthServiceTests PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "/opt/homebrew/lib"
    INSTALL_RPATH_USE_LINK_PATH ON
)

target_include_directories(AuthServiceTests PRIVATE
    ${libsodium_SOURCE_DIR}/src/libsodium/include
    ${libsodium_BINARY_DIR}/src/libsodium/include
)

target_link_libraries(AuthServiceTests PRIVATE ${SODIUM_LIB})

target_link_libraries(AuthServiceTests
    PRIVATE
        AuthServiceCore
        Catch2::Catch2
        Qt6::Widgets
        Qt6::Test
        Qt6::Network
        BackendMocks
        Network
        sodium
)

include(CTest)
enable_testing()
include(Catch)
catch_discover_tests(AuthServiceTests)
