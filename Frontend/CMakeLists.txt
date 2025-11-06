cmake_minimum_required(VERSION 3.10)
project(Frontend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network WebSockets Gui)
find_package(spdlog REQUIRED)

set(FRONTEND_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FRONTEND_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(DEBUG_PROFILING_DIR ${CMAKE_SOURCE_DIR}/DebugProfiling)

add_library(FrontendLib
    ${FRONTEND_SRC_DIR}/presenter.cpp
    ${FRONTEND_SRC_DIR}/model.cpp
    ${FRONTEND_SRC_DIR}/datainputservice.cpp

    ${FRONTEND_SRC_DIR}/models/messagemodel.cpp
    ${FRONTEND_SRC_DIR}/models/chatmodel.cpp
    ${FRONTEND_SRC_DIR}/models/UserModel.cpp

    ${FRONTEND_SRC_DIR}/delegators/messagedelegate.cpp
    ${FRONTEND_SRC_DIR}/delegators/UserDelegate.cpp
    ${FRONTEND_SRC_DIR}/delegators/ChatItemDelegate.cpp
    ${FRONTEND_SRC_DIR}/managers/NetworkAccessManager.cpp

    ${FRONTEND_SRC_DIR}/managers/sessionmanager.cpp

    ${FRONTEND_SRC_DIR}/managers/chatmanager.cpp
    ${FRONTEND_SRC_DIR}/managers/messagemanager.cpp
    ${FRONTEND_SRC_DIR}/managers/usermanager.cpp
    ${FRONTEND_SRC_DIR}/managers/socketmanager.cpp
    ${FRONTEND_SRC_DIR}/managers/BaseManager.cpp
    ${FRONTEND_SRC_DIR}/managers/datamanager.cpp
    ${FRONTEND_SRC_DIR}/forms/mainwindow.ui

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

    ${FRONTEND_INC_DIR}/delegators/messagedelegate.h
    ${FRONTEND_INC_DIR}/delegators/UserDelegate.h
    ${FRONTEND_INC_DIR}/delegators/ChatItemDelegate.h





)

target_include_directories(FrontendLib PUBLIC
    ${FRONTEND_INC_DIR}
    ${FRONTEND_SRC_DIR}
    ${DEBUG_PROFILING_DIR}
    {CMAKE_SOURCE_DIR}/include
    {CMAKE_SOURCE_DIR}/src
    /usr/local/include
    /opt/homebrew/include
)

include_directories(FrontendLib PUBLIC
    ${CMAKE_SOURCE_DIR}
    {CMAKE_SOURCE_DIR}/include
)

target_link_libraries(FrontendLib PUBLIC
    Qt6::Widgets Qt6::Network Qt6::WebSockets Qt6::Gui
    spdlog::spdlog
    redis++
    hiredis
)

target_compile_definitions(FrontendLib PUBLIC SPDLOG_HEADER_ONLY)

add_library(Frontend::Lib ALIAS FrontendLib)

add_executable(Frontend
    ${FRONTEND_SRC_DIR}/main.cpp
    ${FRONTEND_SRC_DIR}/mainwindow.cpp
    ${FRONTEND_INC_DIR}/mainwindow.h
    ${FRONTEND_SRC_DIR}/forms/mainwindow.ui
    config/domains.txt
)

target_link_libraries(Frontend PRIVATE FrontendLib)

add_subdirectory(tests)

set_target_properties(Frontend PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "/usr/local/lib;/opt/homebrew/lib"
    INSTALL_RPATH_USE_LINK_PATH ON
)
