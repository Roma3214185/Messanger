cmake_minimum_required(VERSION 3.22)
project(RabbitMQClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

set(RABBITMQC_DIR ${CMAKE_SOURCE_DIR}/external/rabbitmq-c)
set(SIMPLE_AMQP_CLIENT_DIR ${CMAKE_SOURCE_DIR}/external/SimpleAmqpClient)

add_library(RabbitMQClient STATIC src/rabbitmqclient.cpp)

target_include_directories(RabbitMQClient
    PUBLIC
        /usr/local/include
        /opt/homebrew/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${RABBITMQC_DIR}
        ${SIMPLE_AMQP_CLIENT_DIR}
)

target_link_libraries(RabbitMQClient
    PUBLIC
        SimpleAmqpClient
        /opt/homebrew/lib/librabbitmq.a
        Qt6::Core
        Persistence
        Metrics
)
