cmake_minimum_required(VERSION 3.16)
project(MessageService LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(MESSAGE_SERVICE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MESSAGE_SERVICE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_compile_definitions(QT_NO_KEYWORDS)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../common/Persistence ../../build_message_persistence)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../common/RabbitMQClient ../../build_message_rabbit)

include(FetchContent)

FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.3
)
FetchContent_MakeAvailable(Crow)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)
find_package(nlohmann_json REQUIRED)

link_directories(/opt/homebrew/lib)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(MessageServiceCore STATIC ${MY_SOURCES})

if(ENABLE_CLANG_TIDY)
   enable_clang_tidy(MessageServiceCore)
endif()

if(ENABLE_CPPCHECK)
    enable_cppcheck(MessageServiceCore)
endif()

if(ENABLE_INCLUDE_WHAT_YOU_USE)
    enable_iwyu(MessageServiceCore)
endif()

if(ENABLE_SANITIZER_ADDRESS OR ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
    enable_sanitizers(MessageServiceCore)
endif()

if(ENABLE_CLANG_FORMAT)
    enable_clang_format(MessageServiceCore)
endif()

#enable_project_warnings(MessageServiceCore)

target_link_libraries(MessageServiceCore PUBLIC
  Crow::Crow
  Entities
  RabbitMQClient
  jwt-cpp
  nlohmann_json::nlohmann_json
  Constants
  Network
  Persistence
)

target_include_directories(MessageServiceCore PUBLIC
    ${httplib_SOURCE_DIR}
    ${MESSAGE_SERVICE_INCLUDE_DIR}/
)

add_executable(MessageService
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  Dockerfile
  openapi.yaml
)

target_link_libraries(MessageService PRIVATE MessageServiceCore Qt6::Core)

include(GNUInstallDirs)
install(TARGETS MessageService
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set_target_properties(MessageService PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "@executable_path;@executable_path/../lib;/opt/homebrew/lib;/usr/local/lib"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
