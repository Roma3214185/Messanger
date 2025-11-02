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
set(DEBUG_PROFILING_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../DebugProfiling)

# --- Build a local Frontend library for tests ---
add_library(FrontendLib_local
    ${FRONTEND_SRC_DIR}/Presenter/presenter.cpp
    ${FRONTEND_SRC_DIR}/MessageModel/messagemodel.cpp
    # ${FRONTEND_SRC_DIR}/MessageDelegate/messagemedelegate.cpp
    ${FRONTEND_SRC_DIR}/ChatModel/chatmodel.cpp
    ${FRONTEND_SRC_DIR}/Model/model.cpp
    ${FRONTEND_SRC_DIR}/UserModel/UserModel.cpp
    ${FRONTEND_SRC_DIR}/DataInputService/datainputservice.cpp
    ${FRONTEND_SRC_DIR}/UserDelegate/UserDelegate.cpp
    ${FRONTEND_SRC_DIR}/ChatItemDelegate/ChatItemDelegate.cpp
    ${FRONTEND_SRC_DIR}/NetworkAccessManager/NetworkAccessManager.cpp
    ${FRONTEND_SRC_DIR}/Managers/SessionManager/sessionmanager.cpp
    ${FRONTEND_SRC_DIR}/Managers/ChatManager/chatmanager.cpp
    ${FRONTEND_SRC_DIR}/Managers/MessageManager/messagemanager.cpp
    ${FRONTEND_SRC_DIR}/Managers/UserManager/usermanager.cpp
    ${FRONTEND_SRC_DIR}/Managers/SocketManager/socketmanager.cpp
    ${FRONTEND_SRC_DIR}/Managers/DataManager/datamanager.cpp
    ${FRONTEND_SRC_DIR}/../src/headers/MessageListView.h
    ${FRONTEND_SRC_DIR}/../../DebugProfiling/Debug_profiling.h
)

target_include_directories(FrontendLib_local PUBLIC
    ${FRONTEND_SRC_DIR}
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

# --- Build the test executable ---
add_executable(FrontendServiceTests
    main.cpp
    test_datainputservice.cpp
    test_presenter.cpp
    test_model.cpp
    test_sessionmanager.cpp
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
