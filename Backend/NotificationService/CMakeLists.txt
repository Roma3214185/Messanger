cmake_minimum_required(VERSION 3.16)
project(NotificationService LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(NOTIFICATION_SERVICE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(NOTIFICATION_SERVICE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../common/RabbitMQClient ../../build_notification_rabbit)

find_package(nlohmann_json REQUIRED)

add_compile_definitions(QT_NO_KEYWORDS)

include(FetchContent)

FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.3
)
FetchContent_MakeAvailable(Crow)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(NotificationServiceCore STATIC ${MY_SOURCES})

target_include_directories(NotificationServiceCore PUBLIC
    ${NOTIFICATION_SERVICE_INCLUDE_DIR}
    ${httplib_SOURCE_DIR}
    /opt/homebrew/include
)

target_link_directories(NotificationServiceCore PRIVATE
    /opt/homebrew/lib
)

target_link_libraries(NotificationServiceCore PUBLIC
    Crow::Crow
    nlohmann_json::nlohmann_json
    Metrics
    jwt-cpp
    RabbitMQClient
    Network
    Constants
    Entities
)

add_executable(NotificationService
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    Dockerfile
)

target_link_libraries(NotificationService PRIVATE NotificationServiceCore)

include(GNUInstallDirs)
install(TARGETS NotificationService
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
