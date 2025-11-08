cmake_minimum_required(VERSION 3.16)
project(FrontendServiceTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# --- Find dependencies ---
find_package(Qt6 REQUIRED COMPONENTS Widgets Network WebSockets Gui Test)
find_package(spdlog REQUIRED)
find_package(Catch2 3 REQUIRED)

# --- Frontend source directories ---
set(FRONTEND_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)
set(FRONTEND_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)
set(DEBUG_PROFILING_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../DebugProfiling)

# --- Build a local Frontend library for tests ---
add_library(FrontendLib_local
    ${FRONTEND_SRC_DIR}/presenter.cpp
    ${FRONTEND_SRC_DIR}/models/messagemodel.cpp
    # ${FRONTEND_SRC_DIR}/MessageDelegate/messagemedelegate.cpp
    ${FRONTEND_SRC_DIR}/models/chatmodel.cpp
    ${FRONTEND_SRC_DIR}/model.cpp
    ${FRONTEND_SRC_DIR}/models/UserModel.cpp
    ${FRONTEND_SRC_DIR}/datainputservice.cpp
    ${FRONTEND_SRC_DIR}/delegators/UserDelegate.cpp
    ${FRONTEND_SRC_DIR}/delegators/ChatItemDelegate.cpp
    ${FRONTEND_SRC_DIR}/managers/NetworkAccessManager.cpp
    ${FRONTEND_SRC_DIR}/managers/sessionmanager.cpp
    ${FRONTEND_SRC_DIR}/managers/chatmanager.cpp
    ${FRONTEND_SRC_DIR}/managers/messagemanager.cpp
    ${FRONTEND_SRC_DIR}/managers/usermanager.cpp
    ${FRONTEND_SRC_DIR}/managers/socketmanager.cpp
    ${FRONTEND_SRC_DIR}/managers/datamanager.cpp
    ${FRONTEND_SRC_DIR}/../include/MessageListView.h
    ${FRONTEND_SRC_DIR}/../../DebugProfiling/Debug_profiling.h
    ${FRONTEND_SRC_DIR}/../include/interfaces/ISocket.h
    ${FRONTEND_SRC_DIR}/../src/managers/BaseManager.cpp

    ${FRONTEND_INC_DIR}/managers/BaseManager.h
    ${FRONTEND_INC_DIR}/managers/socketmanager.h
    ${FRONTEND_INC_DIR}/managers/sessionmanager.h
    ${FRONTEND_INC_DIR}/managers/chatmanager.h
    ${FRONTEND_INC_DIR}/managers/usermanager.h
    ${FRONTEND_INC_DIR}/managers/messagemanager.h
    ${FRONTEND_INC_DIR}/MessageListView.h
    ${FRONTEND_INC_DIR}/Model.h
    ${FRONTEND_INC_DIR}/interfaces/ISocket.h
    ${FRONTEND_INC_DIR}/models/ChatModel.h
    ${FRONTEND_INC_DIR}/models/messagemodel.h
    ${FRONTEND_INC_DIR}/models/chatmodel.h
    ${FRONTEND_INC_DIR}/models/UserModel.h
    ${FRONTEND_INC_DIR}/presenter.h
)

target_include_directories(FrontendLib_local PUBLIC
    ${FRONTEND_SRC_DIR}
    ${FRONTEND_INC_DIR}
    ${DEBUG_PROFILING_DIR}
    /usr/local/include
    /opt/homebrew/include
    ${FRONTEND_SRC_DIR}/../../
    ${FRONTEND_SRC_DIR}/../../DebugProfiling
)

target_link_libraries(FrontendLib_local PUBLIC
    Qt6::Widgets Qt6::Network Qt6::WebSockets Qt6::Gui
    spdlog::spdlog
    #redis++
    #hiredis
)

target_compile_definitions(FrontendLib_local PUBLIC SPDLOG_HEADER_ONLY)

add_executable(FrontendServiceTests
    main.cpp
    test_datainputservice.cpp
    test_presenter.cpp
    test_model.cpp
    test_sessionmanager.cpp
    test_socketmanager.cpp
    test_messagemanager.cpp
    test_usermanager.cpp
    test_chatmanager.cpp
)

set_target_properties(FrontendServiceTests PROPERTIES
    AUTOMOC ON
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "/opt/homebrew/lib"
    INSTALL_RPATH_USE_LINK_PATH ON
)

target_link_libraries(FrontendServiceTests
    PRIVATE
        FrontendLib_local
        Catch2::Catch2WithMain
        Qt6::Widgets
        Qt6::Test
)

set_target_properties(FrontendServiceTests PROPERTIES AUTOMOC ON)

include(CTest)
include(Catch)
# catch_discover_tests(FrontendServiceTests)
