cmake_minimum_required(VERSION 3.22)
project(Chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_definitions(-DQT_NO_KEYWORDS -DCROW_USE_ASIO)
#add_compile_definitions(SPDLOG_HEADER_ONLY)

set(BOOST_ROOT "/opt/homebrew/opt/boost@1.89")
set(RABBITMQC_DIR ${CMAKE_SOURCE_DIR}/external/rabbitmq-c)
set(SIMPLE_AMQP_CLIENT_DIR ${CMAKE_SOURCE_DIR}/external/SimpleAmqpClient)
set(DEBUG_PROFILING_DIR ${CMAKE_SOURCE_DIR}/DebugProfiling)
set(CMAKE_OSX_ARCHITECTURES arm64)


find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets WebSockets Gui Sql)
find_package(SQLite3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(ZLIB REQUIRED)

include_directories(
    /usr/local/include
    /opt/homebrew/include
    ${BOOST_ROOT}/include
    ${RABBITMQC_DIR}/include
    ${SIMPLE_AMQP_CLIENT_DIR}/include
    ${DEBUG_PROFILING_DIR}
    ${CMAKE_SOURCE_DIR}/Backend
)

link_directories(
    /usr/local/lib
    /opt/homebrew/lib
    /opt/homebrew/opt/hiredis/lib
    /opt/homebrew/opt/redis-plus-plus/lib
)

add_subdirectory(external/crow)
add_subdirectory(external/jwt-cpp)
add_subdirectory(external/tracy)
add_subdirectory(external/SimpleAmqpClient)

add_library(Metrics STATIC DebugProfiling/metrics.cpp)
target_include_directories(Metrics PUBLIC DebugProfiling)
target_link_libraries(Metrics PUBLIC prometheus-cpp-core prometheus-cpp-pull ZLIB::ZLIB)

add_library(RabbitMQClient STATIC Backend/RabbitMQClient/RabbitMQClient.cpp)
target_include_directories(RabbitMQClient PUBLIC Backend/RabbitMQClient Backend/GenericRepository ${RABBITMQC_DIR}/include ${SIMPLE_AMQP_CLIENT_DIR}/include)
target_link_libraries(RabbitMQClient PRIVATE SimpleAmqpClient rabbitmq)

add_library(RedisCache STATIC Backend/RedisCache/RedisCache.cpp)
target_include_directories(RedisCache PUBLIC Backend/RedisCache ${DEBUG_PROFILING_DIR})
target_link_libraries(RedisCache PUBLIC redis++ hiredis)

add_library(Persistence STATIC
    Backend/Persistence/src/GenericRepository.cpp
    Backend/Persistence/src/SQLiteDatabase.cpp
    Backend/Persistence/src/ThreadPool.cpp
    Backend/Persistence/src/SqlExecutor.cpp
)

target_include_directories(Persistence
    PUBLIC
        ${CMAKE_SOURCE_DIR}/Backend/Persistence/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/Backend/Persistence/inl
)

target_link_libraries(Persistence PUBLIC Qt6::Core Qt6::Sql)

add_subdirectory(Backend/AuthService)
add_subdirectory(Backend/MessageService)
add_subdirectory(Backend/ChatService)
add_subdirectory(Backend/ApigateWay)
add_subdirectory(Backend/NotificationService)
#add_subdirectory(/Users/roma/QtProjects/Chat/prometheus-cpp)

foreach(service IN ITEMS AuthService MessageService ChatService ApiGateWay NotificationService)
    target_link_libraries(${service} PRIVATE RabbitMQClient Persistence RedisCache Metrics prometheus-cpp-core prometheus-cpp-pull)
    target_include_directories(${service}
        PRIVATE ${DEBUG_PROFILING_DIR}
        ${CMAKE_SOURCE_DIR}/Backend/Persistence/include
    )
endforeach()

add_subdirectory(Frontend)

target_link_libraries(Frontend PRIVATE RabbitMQClient Persistence RedisCache prometheus-cpp-core prometheus-cpp-pull)
target_include_directories(Frontend PRIVATE
    ${DEBUG_PROFILING_DIR}
    ${CMAKE_SOURCE_DIR}/Backend/Persistence/include
)

add_subdirectory(Backend/Persistence/benchmarks Backend/Persistence/benchmarks_build)

add_executable(Chat main.cpp media.qrc)

target_link_libraries(Chat PRIVATE
    Qt6::Core Qt6::Network Qt6::Widgets Qt6::WebSockets Qt6::Gui
    SQLite::SQLite3
    redis++ hiredis
    Crow::Crow
    jwt-cpp
    spdlog::spdlog
    Tracy::TracyClient
    RabbitMQClient
    RedisCache
    Persistence
    prometheus-cpp-core prometheus-cpp-pull
)

include_directories(Chat PUBLIC
    ${CMAKE_SOURCE_DIR}
)

target_include_directories(Chat PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DEBUG_PROFILING_DIR}
    ${RABBITMQC_DIR}
    ${SIMPLE_AMQP_CLIENT_DIR}
    ${CMAKE_SOURCE_DIR}/external/jwt-cpp/include
    ${CMAKE_SOURCE_DIR}/Backend
)
