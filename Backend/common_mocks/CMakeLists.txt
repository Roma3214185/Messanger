cmake_minimum_required(VERSION 3.22)
project(BackendMocks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)

add_library(BackendMocks INTERFACE)

target_include_directories(Persistence
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(BackendMocks
    INTERFACE
        Qt6::Core
        Metrics
        RedisCache
        Persistence
        Network
        RabbitMQClient
        Entities
)
