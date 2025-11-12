# Messenger App

A modern, secure, and high-performance C++ messaging application designed for real-time communication. This project focuses on reliability, scalability, and maintainable code architecture.

## Table of Contents
- [Features](#features)
- [Demo](#demo)
- [Installation & Build](#installation--build)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Technologies](#technologies)
- [Contributing](#contributing)
- [Links](#links)

## Features
- Real-time messaging with private and group chats
- User authentication and secure login
- Sending text messages, images, and files
- Notifications for new messages
- Cross-platform support
- Unit tests and code coverage integrated


## Installation & Build
### Prerequisites
- macOS with [Homebrew](https://brew.sh/)
- Git
- CMake and Ninja (installed via Homebrew or vcpkg)
- Qt6, Boost, Catch2, SQLite3, spdlog, hiredis, RabbitMQ-C, ASIO, OpenSSL, nlohmann-json
- `ccache` for faster rebuilds

### Build Steps
Clone the repository:
```bash
git clone --recurse-submodules https://github.com/yourusername/messenger-app.git
cd messenger-app

brew update
brew install cmake ninja qt6 boost catch2 sqlite3 spdlog hiredis rabbitmq-c asio openssl@3 nlohmann-json ccache lcov clang-format
./external/vcpkg/bootstrap-vcpkg.sh
./external/vcpkg/vcpkg install boost-asio spdlog nlohmann-json openssl catch2 hiredis sqlite3

git clone --depth 1 https://github.com/sewenew/redis-plus-plus.git redis-plus-plus
cd redis-plus-plus
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/local
cmake --build . --parallel
cmake --install .
cd ../..

rm -rf build
cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="--coverage -O0" \
    -DCMAKE_EXE_LINKER_FLAGS="--coverage" \
    -DCMAKE_SHARED_LINKER_FLAGS="--coverage" \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_TOOLCHAIN_FILE=$PWD/external/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_PREFIX_PATH="$HOME/local:$CMAKE_PREFIX_PATH" \
    -DREDISPP_INCLUDE_DIR=$HOME/local/include \
    -DREDISPP_LIBRARY=$HOME/local/lib/libredis++.a \
    -DTRACY_ENABLE=ON
cmake --build build --parallel

ctest --test-dir build --output-on-failure

lcov --directory build --capture --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/_deps/*' '*/include/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage-report

codecov -f coverage.info
```

## Usage

The project consists of multiple microservices and a standalone Qt frontend application.  
Each component must be built and launched separately.

### 1. Run Redis and RabbitMQ
Make sure Redis and RabbitMQ servers are running locally (or in Docker):
```bash
brew services start redis
brew services start rabbitmq
```
### 2. Launch Backend Microservices

Each service can be built and started individually from its directory:

#### AuthService
Handles user registration, login, and JWT generation.
```bash
cd Backend/AuthService
./build/AuthService
```

#### ChatService
Manages chat creation, members, and persistence.
```bash
cd Backend/ChatService
./build/ChatService
```

### MessageService
Processes message sending, routing, and delivery.
```bash
cd Backend/MessageService
./build/MessageService
```

### NotificationService
Sends real-time notifications via WebSockets.
```bash
cd Backend/NotificationService
./build/NotificationService
```

### ApiGateway
Acts as the main entry point for frontend requests.
It handles routing, authentication, and load balancing.
```bash
cd Backend/ApigateWay
./build/ApiGateway
```

### 3. Launch Frontend
The Qt-based frontend communicates with the API Gateway.
```bash
cd Frontend
./build/Frontend
```
After launching:
Register or log in.
Send and receive messages in real time.

- Logs: Each service uses spdlog for structured logging.
- Metrics: Prometheus-compatible metrics are exposed via /metrics endpoints.

### 4. Running tests
All tests are integrated with CTest.
You can run them after building the project:
```bash
ctest --test-dir build --output-on-failure
```

## Project Structure
```bash
Directory structure:
└── roma3214185-messanger/
    ├── CMakeLists.txt
    ├── README.md
    ├── fix-submodules.sh
    ├── main.cpp
    ├── media.qrc
    ├── run-clang.sh
    ├── .clang-format
    ├── .clang-tidy
    ├── .clang-tidy.save
    ├── Backend/
    │   ├── ApigateWay/
    │   │   ├── CMakeLists.txt
    │   │   ├── include/
    │   │   │   ├── gatewayserver.h
    │   │   │   ├── JwtUtils.h
    │   │   │   ├── proxyclient.h
    │   │   │   ├── ratelimiter.h
    │   │   │   └── websocketbridge.h
    │   │   └── src/
    │   │       ├── gatewayserver.cpp
    │   │       ├── JwtUtils.cpp
    │   │       ├── main.cpp
    │   │       ├── proxyclient.cpp
    │   │       └── websocketbridge.cpp
    │   ├── AuthService/
    │   │   ├── CMakeLists.txt
    │   │   ├── include/
    │   │   │   ├── authcontroller.h
    │   │   │   ├── authmanager.h
    │   │   │   ├── JwtUtils.h
    │   │   │   ├── PasswordService.h
    │   │   │   ├── server.h
    │   │   │   └── entities/
    │   │   │       ├── AuthResponce.h
    │   │   │       └── RegisterRequest.h
    │   │   └── src/
    │   │       ├── authcontroller.cpp
    │   │       ├── authmanager.cpp
    │   │       ├── JwtUtils.cpp
    │   │       ├── main.cpp
    │   │       └── server.cpp
    │   ├── ChatService/
    │   │   ├── CMakeLists.txt
    │   │   ├── include/
    │   │   │   ├── chatmanager.h
    │   │   │   ├── controller.h
    │   │   │   ├── database.h
    │   │   │   ├── NetworkManager.h
    │   │   │   ├── server.h
    │   │   │   └── TokenService.h
    │   │   └── src/
    │   │       ├── chatmanager.cpp
    │   │       ├── controller.cpp
    │   │       ├── database.cpp
    │   │       ├── main.cpp
    │   │       ├── NetworkManager.cpp
    │   │       └── server.cpp
    │   ├── MessageService/
    │   │   ├── CMakeLists.txt
    │   │   ├── include/
    │   │   │   ├── controller.h
    │   │   │   ├── messageworker.h
    │   │   │   ├── server.h
    │   │   │   └── managers/
    │   │   │       ├── JwtUtils.h
    │   │   │       ├── MessageManager.h
    │   │   │       ├── NetworkManager.h
    │   │   │       └── notificationmanager.h
    │   │   └── src/
    │   │       ├── controller.cpp
    │   │       ├── JwtUtils.cpp
    │   │       ├── main.cpp
    │   │       ├── messageworker.cpp
    │   │       ├── server.cpp
    │   │       └── managers/
    │   │           ├── MessageManager.cpp
    │   │           ├── NetworkManager.cpp
    │   │           └── notificationmanager.cpp
    │   ├── NotificationService/
    │   │   ├── CMakeLists.txt
    │   │   ├── include/
    │   │   │   ├── JwtUtils.h
    │   │   │   ├── server.h
    │   │   │   ├── entities/
    │   │   │   │   ├── Message.h
    │   │   │   │   └── MessageStatus.h
    │   │   │   └── managers/
    │   │   │       ├── networkmanager.h
    │   │   │       ├── notificationmanager.h
    │   │   │       └── SocketManager.h
    │   │   └── src/
    │   │       ├── main.cpp
    │   │       ├── server.cpp
    │   │       └── managers/
    │   │           ├── networkmanager.cpp
    │   │           ├── notificationmanager.cpp
    │   │           └── socketmanager.cpp
    │   └── shared_keys/
    │       └── public_key.pem
    ├── common/
    │   ├── entities/
    │   │   ├── CMakeLists.txt
    │   │   └── include/
    │   │       └── entities/
    │   │           ├── Chat.h
    │   │           ├── ChatMember.h
    │   │           ├── Message.h
    │   │           ├── MessageStatus.h
    │   │           ├── User.h
    │   │           └── UserCredentials.h
    │   ├── Metrics/
    │   │   ├── CMakeLists.txt
    │   │   ├── include/
    │   │   │   ├── Debug_profiling.h
    │   │   │   ├── metrics.h
    │   │   │   └── ScopedRequestsTimer.h
    │   │   └── src/
    │   │       └── metrics.cpp
    │   ├── Persistence/
    │   │   ├── CMakeLists.txt
    │   │   ├── benchmarks/
    │   │   │   ├── README.md
    │   │   │   ├── batcher_benchmark.cpp
    │   │   │   ├── benchmark_entity.cpp
    │   │   │   ├── benchmark_latency.cpp
    │   │   │   ├── benchmarks_query.cpp
    │   │   │   ├── build_entity.cpp
    │   │   │   ├── CMakeLists.txt
    │   │   │   ├── main.cpp
    │   │   │   ├── preparedStatements_benchmark.cpp
    │   │   │   ├── redis_cache_benchmark.cpp
    │   │   │   └── thread_pool_benchmark.cpp
    │   │   ├── benchmarks_build/
    │   │   │   └── cmake_install.cmake
    │   │   ├── include/
    │   │   │   ├── Batcher.h
    │   │   │   ├── GenericRepository.h
    │   │   │   ├── Meta.h
    │   │   │   ├── Query.h
    │   │   │   ├── SqlBuilder.h
    │   │   │   ├── SqlExecutor.h
    │   │   │   ├── SQLiteDataBase.h
    │   │   │   ├── ThreadPool.h
    │   │   │   └── interfaces/
    │   │   │       ├── BaseQuery.h
    │   │   │       ├── IDataBase.h
    │   │   │       ├── IEntityBuilder.h
    │   │   │       └── ISqlExecutor.h
    │   │   ├── inl/
    │   │   │   ├── GenericRepository.inl
    │   │   │   ├── Query.inl
    │   │   │   ├── SqlBuilder.inl
    │   │   │   └── ThreadPool.inl
    │   │   ├── src/
    │   │   │   ├── GenericRepository.cpp
    │   │   │   ├── Query.cpp
    │   │   │   ├── SqlExecutor.cpp
    │   │   │   ├── SQLiteDatabase.cpp
    │   │   │   └── ThreadPool.cpp
    │   │   └── tests/
    │   │       ├── CMakeLists.txt
    │   │       ├── test_genericrepository.cpp
    │   │       ├── test_query.cpp
    │   │       └── mocks/
    │   │           ├── FakeSqlExecutor.h
    │   │           ├── main.cpp
    │   │           ├── MockCache.h
    │   │           ├── MockDatabase.h
    │   │           └── MockQuery.h
    │   ├── RabbitMQClient/
    │   │   ├── CMakeLists.txt
    │   │   ├── include/
    │   │   │   └── RabbitMQClient.h
    │   │   └── src/
    │   │       └── rabbitmqclient.cpp
    │   └── RedisCache/
    │       ├── CMakeLists.txt
    │       ├── include/
    │       │   ├── RedisCache.h
    │       │   └── interfaces/
    │       │       └── ICacheService.h
    │       └── src/
    │           └── RedisCache.cpp
    ├── Frontend/
    │   ├── cmake_install.cmake
    │   ├── CMakeLists.txt
    │   ├── config/
    │   │   └── domains.txt
    │   ├── forms/
    │   │   └── mainwindow.ui
    │   ├── include/
    │   │   ├── DataInputService.h
    │   │   ├── JsonService.h
    │   │   ├── mainwindow.h
    │   │   ├── MessageListView.h
    │   │   ├── model.h
    │   │   ├── presenter.h
    │   │   ├── RealSocket.h
    │   │   ├── RedisClient.h
    │   │   ├── delegators/
    │   │   │   ├── chatitemdelegate.h
    │   │   │   ├── messagedelegate.h
    │   │   │   └── userdelegate.h
    │   │   ├── dto/
    │   │   │   ├── ChatBase.h
    │   │   │   ├── DrawData.h
    │   │   │   ├── Message.h
    │   │   │   ├── SignUpRequest.h
    │   │   │   └── User.h
    │   │   ├── interfaces/
    │   │   │   ├── ICache.h
    │   │   │   ├── IMainWindow.h
    │   │   │   ├── INetworkAccessManager.h
    │   │   │   ├── ISocket.h
    │   │   │   └── ITheme.h
    │   │   ├── managers/
    │   │   │   ├── BaseManager.h
    │   │   │   ├── chatmanager.h
    │   │   │   ├── datamanager.h
    │   │   │   ├── messagemanager.h
    │   │   │   ├── networkaccessmanager.h
    │   │   │   ├── sessionmanager.h
    │   │   │   ├── socketmanager.h
    │   │   │   └── usermanager.h
    │   │   └── models/
    │   │       ├── chatmodel.h
    │   │       ├── messagemodel.h
    │   │       └── UserModel.h
    │   ├── src/
    │   │   ├── datainputservice.cpp
    │   │   ├── main.cpp
    │   │   ├── mainwindow.cpp
    │   │   ├── model.cpp
    │   │   ├── presenter.cpp
    │   │   ├── delegators/
    │   │   │   ├── ChatItemDelegate.cpp
    │   │   │   ├── messagedelegate.cpp
    │   │   │   └── UserDelegate.cpp
    │   │   ├── managers/
    │   │   │   ├── BaseManager.cpp
    │   │   │   ├── chatmanager.cpp
    │   │   │   ├── datamanager.cpp
    │   │   │   ├── messagemanager.cpp
    │   │   │   ├── NetworkAccessManager.cpp
    │   │   │   ├── sessionmanager.cpp
    │   │   │   ├── socketmanager.cpp
    │   │   │   └── usermanager.cpp
    │   │   └── models/
    │   │       ├── chatmodel.cpp
    │   │       ├── messagemodel.cpp
    │   │       └── UserModel.cpp
    │   └── tests/
    │       ├── cmake_install.cmake
    │       ├── CMakeLists.txt
    │       ├── CTestTestfile.cmake
    │       ├── DartConfiguration.tcl
    │       ├── main.cpp
    │       ├── test_chatmanager.cpp
    │       ├── test_datainputservice.cpp
    │       ├── test_messagemanager.cpp
    │       ├── test_model.cpp
    │       ├── test_presenter.cpp
    │       ├── test_sessionmanager.cpp
    │       ├── test_socketmanager.cpp
    │       ├── test_usermanager.cpp
    │       └── mocks/
    │           ├── FakeSocket.h
    │           ├── MockAccessManager.h
    │           ├── MockCache.h
    │           └── MockReply.h
    ├── .github/
    │   └── workflows/
    │       └── ci.yml
    └── .qt/
        ├── QtDeploySupport.cmake
        └── QtDeployTargets.cmake
```


## Technologies
- **Language:** C++17 / C++20  
- **GUI:** Qt6  
- **Networking:** ASIO, RabbitMQ, Redis++  
- **Database:** SQLite3  
- **Logging:** spdlog  
- **Testing & Coverage:** Catch2, lcov, Codecov  
- **Build System:** CMake + Ninja  
- **Code Formatting:** clang-format, clang-tidy, cpplint

## Links
- **C++ CI:**(https://github.com/roma3214185/messanger/actions/workflows/ci.yml/badge.svg)
- **Code Coverage (Codecov):** [https://app.codecov.io/github/roma3214185/messanger]

