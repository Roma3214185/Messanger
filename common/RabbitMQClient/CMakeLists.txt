cmake_minimum_required(VERSION 3.22)
project(RabbitMQClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT TARGET RabbitMQClient)

find_package(rabbitmq-c REQUIRED CONFIG)

add_library(RabbitMQClient STATIC
    src/rabbitmqclient.cpp)

target_include_directories(RabbitMQClient
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/external/SimpleAmqpClient/src
)

target_link_libraries(RabbitMQClient
    PUBLIC
        SimpleAmqpClient
        rabbitmq::rabbitmq
        ThreadPool
        Metrics
)

endif()
