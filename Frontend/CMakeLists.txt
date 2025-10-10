cmake_minimum_required(VERSION 3.10)
project(Frontend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network WebSockets Gui)
#find_package(spdlog CONFIG REQUIRED)

enable_testing()
set(FRONTEND_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(tests)

add_executable(Frontend
   main.cpp
   src/MainWindow/mainwindow.cpp
   src/MainWindow/mainwindow.h
   src/MainWindow/mainwindow.ui
   src/headers/SignUpRequest.h
   src/Presenter/presenter.h src/Presenter/presenter.cpp
   src/headers/IMainWindow.h
   src/Model/model.cpp
   src/Model/model.h
   src/headers/User.h
   src/MessageDelegate/messagedelegate.cpp
   src/MessageDelegate/messagedelegate.h
   src/MessageModel/messagemodel.cpp
   src/MessageModel/messagemodel.h
   src/ChatItemDelegate/chatitemdelegate.cpp
   src/ChatItemDelegate/chatitemdelegate.h
   src/ChatModel/chatmodel.cpp
   src/ChatModel/chatmodel.h
   src/headers/JsonServer.h
   src/UserModel/UserModel.h
   src/UserDelegate/userdelegate.h src/UserDelegate/userdelegate.cpp
   src/headers/INetworkAccessManager.h
   src/NetworkAccessManager/networkaccessmanager.h src/NetworkAccessManager/networkaccessmanager.cpp
   src/NetworkAccessManager/MockAccessManager.h
   src/headers/ICash.h
   src/headers/RedisClient.h
   src/headers/MockCash.h
   src/UserModel/UserModel.cpp
   src/headers/MockReply.h
   src/headers/FakeSocket.h
   src/headers/Message.h
   src/DataInputService/datainputservice.h
   src/DataInputService/datainputservice.cpp
   src/headers/ChatBase.h
)

target_include_directories(Frontend PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/src
   /usr/local/include
   /opt/homebrew/include
)

target_link_libraries(Frontend PRIVATE
   Qt6::Widgets Qt6::Network Qt6::WebSockets Qt6::Gui
   redis++
   #hiredis
   #spdlog::spdlog
)

set_target_properties(Frontend PROPERTIES
    BUILD_WITH_INSTALL_RPATH ON
    INSTALL_RPATH "/usr/local/lib;/opt/homebrew/lib"
    INSTALL_RPATH_USE_LINK_PATH ON
)
