cmake_minimum_required(VERSION 3.16)
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

add_executable(Chat
    media.qrc
    main.cpp
    .gitignore
)

add_subdirectory(Backend/AuthService)
add_subdirectory(Backend/MessageService)
add_subdirectory(Backend/ChatService)
add_subdirectory(Backend/ApigateWay)
add_subdirectory(Frontend)
add_subdirectory(external/crow)
add_subdirectory(external/jwt-cpp)

target_link_libraries(Chat
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    SQLite::SQLite3
    Crow::Crow
    jwt-cpp
)


target_link_libraries(AuthService PRIVATE  Qt6::Core jwt-cpp SQLite::SQLite3)
target_link_libraries(ChatService PRIVATE )
target_link_libraries(MessageService PRIVATE )

target_include_directories(ChatService PRIVATE /usr/local/include /opt/homebrew/include)
target_include_directories(AuthService PRIVATE /usr/local/include /opt/homebrew/include)
target_include_directories(MessageService PRIVATE /usr/local/include /opt/homebrew/include)
target_include_directories(Frontend PRIVATE /usr/local/include /opt/homebrew/include)

include(GNUInstallDirs)
install(TARGETS Chat
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
