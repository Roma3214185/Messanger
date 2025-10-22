cmake_minimum_required(VERSION 3.16...3.29)
project(Chat LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DCROW_USE_ASIO)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BOOST_ROOT "/opt/homebrew/opt/boost@1.89")
include_directories("${BOOST_ROOT}/include")
add_compile_definitions(SPDLOG_HEADER_ONLY)
#add_compile_definitions(ENABLE_TRACY=1)

include_directories(
    /usr/local/include
    /opt/homebrew/include
)
link_directories(
    /usr/local/lib
    /opt/homebrew/lib
)

find_package(Qt6 REQUIRED COMPONENTS Core Network)
find_package(SQLite3 REQUIRED)
find_package(spdlog REQUIRED)
#find_package(fmt REQUIRED)

add_executable(Chat
    media.qrc
    main.cpp
    .gitignore

    Backend/RedisCashe/RedisCache.h
    Backend/GenericRepository/GenericReposiroty.h

    DebugProfiling/Debug_profiling.h
    Backend/GenericRepository/Query.h
    Backend/GenericRepository/Query.cpp
    Backend/GenericRepository/Meta.h
    Backend/GenericRepository/SQLiteDataBase.h
    Backend/GenericRepository/ThreadPool.h
)



add_subdirectory(Backend/AuthService)
add_subdirectory(Backend/MessageService)
add_subdirectory(Backend/ChatService)
add_subdirectory(Backend/ApigateWay)
add_subdirectory(Frontend)
add_subdirectory(external/crow)
add_subdirectory(external/jwt-cpp)
add_subdirectory(external/tracy)
add_subdirectory(Backend/GenericRepository/benchmarks)

target_link_libraries(Chat
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    SQLite::SQLite3
    redis++
    Crow::Crow
    jwt-cpp
    spdlog::spdlog
    #fmt::fmt
    Tracy::TracyClient
)


target_link_libraries(AuthService PRIVATE  Qt6::Core jwt-cpp SQLite::SQLite3 Crow::Crow)
target_link_libraries(ChatService PRIVATE )
target_link_libraries(MessageService PRIVATE )
target_link_libraries(Frontend PRIVATE TracyClient)

target_include_directories(ChatService PRIVATE /usr/local/include /opt/homebrew/include)
target_include_directories(AuthService PRIVATE /usr/local/include /opt/homebrew/include)
target_include_directories(MessageService PRIVATE /usr/local/include /opt/homebrew/include)
target_include_directories(Frontend PRIVATE /usr/local/include /opt/homebrew/include)
target_include_directories(benchmarks PRIVATE /usr/local/include /opt/homebrew/include)

target_compile_definitions(Frontend PRIVATE SPDLOG_HEADER_ONLY)
target_compile_definitions(AuthService PRIVATE SPDLOG_HEADER_ONLY)
target_compile_definitions(MessageService PRIVATE SPDLOG_HEADER_ONLY)
target_compile_definitions(ChatService PRIVATE SPDLOG_HEADER_ONLY)

include(GNUInstallDirs)
install(TARGETS Chat
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
