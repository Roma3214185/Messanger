cmake_minimum_required(VERSION 3.22)
project(Frontend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network WebSockets Gui)

set(FRONTEND_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(FRONTEND_FORMS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/forms)
set(FRONTEND_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE MY_INCLUDES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

list(FILTER MY_SOURCES EXCLUDE REGEX ".*/mainwindow\\.cpp$")
list(FILTER MY_INCLUDES EXCLUDE REGEX ".*/mainwindow\\.h$")

add_library(FrontendLib STATIC
    ${MY_SOURCES}
    ${MY_INCLUDES}
)

if(ENABLE_CLANG_TIDY)
    enable_clang_tidy(FrontendLib)
endif()

if(ENABLE_CPPCHECK)
    enable_cppcheck(FrontendLib)
endif()

if(ENABLE_INCLUDE_WHAT_YOU_USE)
    enable_iwyu(FrontendLib)
endif()

if(ENABLE_SANITIZER_ADDRESS OR ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
    enable_sanitizers(FrontendLib)
endif()

if(ENABLE_CLANG_FORMAT)
    enable_clang_format(FrontendLib)
endif()


target_include_directories(FrontendLib
    PUBLIC
        ${FRONTEND_INC_DIR}
        ${FRONTEND_FORMS_DIR}
)

target_link_libraries(FrontendLib PUBLIC
    Qt6::Widgets Qt6::Network Qt6::WebSockets Qt6::Gui
    ProjectOptions
    Metrics
    RedisCache
    Entities
)

target_compile_definitions(FrontendLib PUBLIC SPDLOG_HEADER_ONLY)
target_compile_definitions(FrontendLib
    PRIVATE CONFIG_DOMAINS_PATH="${CMAKE_SOURCE_DIR}/Frontend/config/domains.txt"
)

add_library(Frontend::Lib ALIAS FrontendLib)

add_executable(Frontend
    main.cpp
    ${FRONTEND_INC_DIR}/mainwindow.h
    ${FRONTEND_SRC_DIR}/mainwindow.cpp
    ${FRONTEND_FORMS_DIR}/mainwindow.ui
    src/MessageListView.cpp
    src/TokenFactory.cpp
    src/RealSocket.cpp
    src/RedisClient.cpp
    src/utilsui.cpp
    src/utils.cpp
    src/Themes.cpp
)

add_custom_command(TARGET Frontend POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Frontend/config
        $<TARGET_FILE_DIR:Frontend>/config
)

target_link_libraries(Frontend PRIVATE FrontendLib Metrics)

target_include_directories(Frontend PRIVATE ${FRONTEND_FORMS_DIR})
