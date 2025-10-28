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
set(DEBUG_PROFILING_DIR ${CMAKE_SOURCE_DIR}/DebugProfiling)

add_library(FrontendLib
    ${FRONTEND_SRC_DIR}/Presenter/presenter.cpp
    ${FRONTEND_SRC_DIR}/MessageModel/messagemodel.cpp
    ${FRONTEND_SRC_DIR}/MessageDelegate/messagedelegate.cpp
    ${FRONTEND_SRC_DIR}/ChatModel/chatmodel.cpp
    ${FRONTEND_SRC_DIR}/Model/model.cpp
    ${FRONTEND_SRC_DIR}/UserModel/UserModel.cpp
    ${FRONTEND_SRC_DIR}/DataInputService/datainputservice.cpp

    ${FRONTEND_SRC_DIR}/headers/MessageListView.h
    ${FRONTEND_SRC_DIR}/UserDelegate/UserDelegate.cpp
    ${FRONTEND_SRC_DIR}/ChatItemDelegate/ChatItemDelegate.cpp
    ${FRONTEND_SRC_DIR}/NetworkAccessManager/NetworkAccessManager.cpp
)

target_include_directories(FrontendLib PUBLIC
    ${FRONTEND_SRC_DIR}
    ${DEBUG_PROFILING_DIR}
    /usr/local/include
    /opt/homebrew/include
)

include_directories(FrontendLib PUBLIC
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(FrontendLib PUBLIC
    Qt6::Widgets Qt6::Network Qt6::WebSockets Qt6::Gui
    spdlog::spdlog
    redis++
    hiredis
)

target_compile_definitions(FrontendLib PUBLIC SPDLOG_HEADER_ONLY)

add_executable(Frontend
    main.cpp
    ${FRONTEND_SRC_DIR}/MainWindow/mainwindow.cpp
    ${FRONTEND_SRC_DIR}/MainWindow/mainwindow.h
    ${FRONTEND_SRC_DIR}/MainWindow/mainwindow.ui
)

target_link_libraries(Frontend PRIVATE FrontendLib)

set_target_properties(Frontend PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "/usr/local/lib;/opt/homebrew/lib"
    INSTALL_RPATH_USE_LINK_PATH ON
)
