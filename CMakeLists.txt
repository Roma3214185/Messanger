cmake_minimum_required(VERSION 3.22)
project(Chat)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_definitions(-DQT_NO_KEYWORDS -DCROW_USE_ASIO)

#3-rd libraries
add_subdirectory(external/crow)
add_subdirectory(external/jwt-cpp)
add_subdirectory(external/tracy)
add_subdirectory(external/rabbitmq-c)
add_subdirectory(external/SimpleAmqpClient)
add_subdirectory(external/IXWebSocket)
add_subdirectory(external/prometheus-cpp)

# my libraries
add_subdirectory(common/Entities build_entites)
add_subdirectory(common/Metrics build_metrics)
add_subdirectory(common/RedisCache build_redis)
add_subdirectory(common/RabbitMQClient build_rabbit)
add_subdirectory(common/Persistence build_persistence)

# microservices
add_subdirectory(Backend/AuthService)
add_subdirectory(Backend/MessageService)
add_subdirectory(Backend/ChatService)
add_subdirectory(Backend/ApiGateway)
add_subdirectory(Backend/NotificationService)

# frontend
add_subdirectory(Frontend)

# tests
include(CTest)
enable_testing()
add_subdirectory(common/Persistence/tests)
add_subdirectory(Frontend/tests)

