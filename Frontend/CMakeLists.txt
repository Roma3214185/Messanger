cmake_minimum_required(VERSION 3.22)
project(Frontend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network WebSockets Gui)

set(FRONTEND_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FRONTEND_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(FRONTEND_FORMS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/forms)
set(DEBUG_PROFILING_DIR ${CMAKE_SOURCE_DIR}/DebugProfiling)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

#include(${CMAKE_SOURCE_DIR}/cmake/staticanalyzers.cmake)

add_library(FrontendLib STATIC
    ${MY_SOURCES}

    ${FRONTEND_FORMS_DIR}/mainwindow.ui

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

    ${FRONTEND_INC_DIR}/usecases/chatusecase.h
    ${FRONTEND_INC_DIR}/usecases/messageusecase.h
    ${FRONTEND_INC_DIR}/usecases/sessionusecase.h
    ${FRONTEND_INC_DIR}/usecases/userusecase.h
    ${FRONTEND_INC_DIR}/usecases/socketusecase.h

    ${FRONTEND_INC_DIR}/managers/datamanager.h

    ${FRONTEND_INC_DIR}/delegators/messagedelegate.h
    ${FRONTEND_INC_DIR}/delegators/UserDelegate.h
    ${FRONTEND_INC_DIR}/delegators/ChatItemDelegate.h
    ${FRONTEND_INC_DIR}/interfaces/IMessageListView.h
)

enable_clang_tidy(FrontendLib)
enable_cppcheck(FrontendLib)
enable_iwyu(FrontendLib)
enable_sanitizers(FrontendLib)

target_include_directories(FrontendLib
    PUBLIC
        ${FRONTEND_INC_DIR}
        ${FRONTEND_SRC_DIR}
        ${FRONTEND_FORMS_DIR}
        ${DEBUG_PROFILING_DIR}
)

target_link_libraries(FrontendLib PUBLIC
    Qt6::Widgets Qt6::Network Qt6::WebSockets Qt6::Gui
    Metrics
    RedisCache
)

target_compile_definitions(FrontendLib PUBLIC SPDLOG_HEADER_ONLY)
target_compile_definitions(FrontendLib
    PRIVATE CONFIG_DOMAINS_PATH="${CMAKE_SOURCE_DIR}/Frontend/config/domains.txt"
)

add_library(Frontend::Lib ALIAS FrontendLib)

add_executable(Frontend
    ${FRONTEND_SRC_DIR}/main.cpp
    ${FRONTEND_SRC_DIR}/mainwindow.cpp
    ${FRONTEND_INC_DIR}/mainwindow.h
    ${FRONTEND_FORMS_DIR}/mainwindow.ui
    Dockerfile
    include/DeleteMessageResponce.h
)

add_custom_command(TARGET Frontend POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Frontend/config
        $<TARGET_FILE_DIR:Frontend>/config
)

target_link_libraries(Frontend PRIVATE FrontendLib Metrics)

# set_target_properties(Frontend PROPERTIES
#     BUILD_WITH_INSTALL_RPATH ON
#     INSTALL_RPATH "/usr/local/lib;/opt/homebrew/lib"
#     INSTALL_RPATH_USE_LINK_PATH ON
# )
