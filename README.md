# Messenger App

A modern, secure, and high-performance C++ messaging application designed for real-time communication. This project focuses on reliability, scalability, and maintainable code architecture.

## Table of Contents
- [Features](#features)
- [Installation & Build](#installation--build)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Technologies](#technologies)
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
git clone --recurse-submodules https://github.com/Roma3214185/Messanger.git
cd Messanger

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
    ├── README.md
    ├── CMakeLists.txt
    ├── coverage.info
    ├── run-clang.sh
    ├── .clang-tidy.save
    ├── .lcovrc
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
    │   │   │   ├── authservice/
    │   │   │   │   ├── authcontroller.h
    │   │   │   │   ├── authDataInputService.h
    │   │   │   │   ├── authmanager.h
    │   │   │   │   ├── JwtGenerator.h
    │   │   │   │   ├── JwtUtils.h
    │   │   │   │   ├── PasswordService.h
    │   │   │   │   ├── RealAuthoritizer.h
    │   │   │   │   ├── server.h
    │   │   │   │   └── interfaces/
    │   │   │   │       ├── IAuthManager.h
    │   │   │   │       └── IGenerator.h
    │   │   │   └── entities/
    │   │   │       ├── AuthResponce.h
    │   │   │       └── RegisterRequest.h
    │   │   ├── src/
    │   │   │   ├── authcontroller.cpp
    │   │   │   ├── authDataInputService.cpp
    │   │   │   ├── authmanager.cpp
    │   │   │   ├── JwtUtils.cpp
    │   │   │   ├── main.cpp
    │   │   │   └── server.cpp
    │   │   └── tests/
    │   │       ├── CMakeLists.txt
    │   │       ├── main.cpp
    │   │       ├── test_authmanager.cpp
    │   │       ├── test_authserver.cpp
    │   │       ├── test_protected_authmanager.cpp
    │   │       └── mocks/
    │   │           ├── MockAuthManager.h
    │   │           └── MockGenerator.h
    │   ├── ChatService/
    │   │   ├── CMakeLists.txt
    │   │   ├── include/
    │   │   │   └── chatservice/
    │   │   │       ├── AutoritizerProvider.h
    │   │   │       ├── chatcontroller.h
    │   │   │       ├── chatmanager.h
    │   │   │       ├── chatserver.h
    │   │   │       ├── TokenService.h
    │   │   │       └── interfaces/
    │   │   │           └── IChatManager.h
    │   │   ├── src/
    │   │   │   ├── chatcontroller.cpp
    │   │   │   ├── chatmanager.cpp
    │   │   │   ├── chatserver.cpp
    │   │   │   ├── main.cpp
    │   │   │   └── TokenService.cpp
    │   │   └── tests/
    │   │       ├── CMakeLists.txt
    │   │       ├── main.cpp
    │   │       ├── test_chatmanager.cpp
    │   │       ├── test_chatserver.cpp
    │   │       ├── test_controller.cpp
    │   │       └── mocks/
    │   │           └── MockChatManager.h
    │   ├── common_mocks/
    │   │   ├── CMakeLists.txt
    │   │   └── mocks/
    │   │       ├── FakeSqlExecutor.h
    │   │       ├── MockAppWrapper.h
    │   │       ├── MockAutoritizer.h
    │   │       ├── MockCache.h
    │   │       ├── MockConfigProvider.h
    │   │       ├── MockDatabase.h
    │   │       ├── MockNetworkManager.h
    │   │       ├── MockQuery.h
    │   │       ├── MockRabitMQClient.h
    │   │       ├── MockTheadPool.h
    │   │       └── MockUtils.h
    │   ├── MessageService/
    │   │   ├── CMakeLists.txt
    │   │   ├── include/
    │   │   │   └── messageservice/
    │   │   │       ├── controller.h
    │   │   │       ├── server.h
    │   │   │       ├── dto/
    │   │   │       │   └── GetMessagePack.h
    │   │   │       ├── interfaces/
    │   │   │       │   └── IController.h
    │   │   │       └── managers/
    │   │   │           ├── JwtUtils.h
    │   │   │           └── MessageManager.h
    │   │   ├── src/
    │   │   │   ├── controller.cpp
    │   │   │   ├── JwtUtils.cpp
    │   │   │   ├── main.cpp
    │   │   │   ├── server.cpp
    │   │   │   └── managers/
    │   │   │       └── MessageManager.cpp
    │   │   └── tests/
    │   │       ├── CMakeLists.txt
    │   │       ├── main.cpp
    │   │       ├── test_controller.cpp
    │   │       ├── test_messagemanager.cpp
    │   │       ├── test_server.cpp
    │   │       └── mocks/
    │   │           ├── MockController.h
    │   │           ├── SecondTestController.h
    │   │           └── TestController.h
    │   ├── NotificationService/
    │   │   ├── CMakeLists.txt
    │   │   ├── include/
    │   │   │   ├── entities/
    │   │   │   │   ├── Message.h
    │   │   │   │   └── MessageStatus.h
    │   │   │   ├── interfaces/
    │   │   │   │   └── ISocket.h
    │   │   │   └── notificationservice/
    │   │   │       ├── CrowSocket.h
    │   │   │       ├── JwtUtils.h
    │   │   │       ├── server.h
    │   │   │       └── managers/
    │   │   │           ├── notificationmanager.h
    │   │   │           └── socketmanager.h
    │   │   ├── src/
    │   │   │   ├── main.cpp
    │   │   │   ├── server.cpp
    │   │   │   └── managers/
    │   │   │       ├── notificationmanager.cpp
    │   │   │       └── socketmanager.cpp
    │   │   └── tests/
    │   │       ├── CMakeLists.txt
    │   │       ├── main.cpp
    │   │       ├── test_notificationmanager.cpp
    │   │       ├── test_server.cpp
    │   │       └── mocks/
    │   │           └── MockSocket.h
    │   └── shared_keys/
    │       └── public_key.pem
    ├── common/
    │   ├── constants/
    │   │   ├── CMakeLists.txt
    │   │   └── include/
    │   │       ├── codes.h
    │   │       ├── ports.h
    │   │       ├── ProdConfigProvider.h
    │   │       ├── Routes.h
    │   │       └── interfaces/
    │   │           └── IConfigProvider.h
    │   ├── entities/
    │   │   ├── CMakeLists.txt
    │   │   └── include/
    │   │       ├── Fields.h
    │   │       └── entities/
    │   │           ├── Chat.h
    │   │           ├── ChatMember.h
    │   │           ├── Message.h
    │   │           ├── MessageStatus.h
    │   │           ├── PrivateChat.h
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
    │   ├── Network/
    │   │   ├── CMakeLists.txt
    │   │   ├── include/
    │   │   │   ├── NetworkFacade.h
    │   │   │   ├── NetworkManager.h
    │   │   │   ├── RealCrowApp.h
    │   │   │   └── interfaces/
    │   │   │       ├── IApp.h
    │   │   │       ├── IAutoritizer.h
    │   │   │       ├── IChatNetworkManager.h
    │   │   │       ├── IMessageNetworkManager.h
    │   │   │       ├── INetworkManagerBase.h
    │   │   │       └── IUserNetworkManager.h
    │   │   └── src/
    │   │       ├── IChatNetworkManager.cpp
    │   │       ├── IMessageNetworkManager.cpp
    │   │       ├── INetworkManagerBase.cpp
    │   │       └── IUserNetworkManager.cpp
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
    │   │   │       ├── ISqlExecutor.h
    │   │   │       └── IThreadPool.h
    │   │   ├── inl/
    │   │   │   ├── GenericRepository.inl
    │   │   │   ├── Query.inl
    │   │   │   └── SqlBuilder.inl
    │   │   ├── src/
    │   │   │   ├── GenericRepository.cpp
    │   │   │   ├── Query.cpp
    │   │   │   ├── SqlExecutor.cpp
    │   │   │   ├── SQLiteDatabase.cpp
    │   │   │   └── ThreadPool.cpp
    │   │   └── tests/
    │   │       ├── CMakeLists.txt
    │   │       ├── main.cpp
    │   │       ├── test_genericrepository.cpp
    │   │       ├── test_query.cpp
    │   │       └── test_sqlitedatabase.cpp
    │   ├── RabbitMQClient/
    │   │   ├── CMakeLists.txt
    │   │   ├── include/
    │   │   │   ├── RabbitMQClient.h
    │   │   │   └── interfaces/
    │   │   │       └── IRabitMQClient.h
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
    │   │   │   ├── IMessageListView.h
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
    │   │   ├── IMessageListView.cpp
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
    │       ├── test_chatitemdelegate.cpp
    │       ├── test_chatmanager.cpp
    │       ├── test_datainputservice.cpp
    │       ├── test_datamanager.cpp
    │       ├── test_messagemanager.cpp
    │       ├── test_model.cpp
    │       ├── test_presenter.cpp
    │       ├── test_sessionmanager.cpp
    │       ├── test_socketmanager.cpp
    │       ├── test_usermanager.cpp
    │       ├── test_usermodel.cpp
    │       └── mocks/
    │           ├── FakeSocket.h
    │           ├── MockAccessManager.h
    │           ├── MockCache.h
    │           ├── MockMainWindow.h
    │           ├── MockMessageListView.h
    │           └── MockReply.h
    ├── Testing/
    │   └── Temporary/
    │       └── CTestCostData.txt
    ├── .github/
    │   └── workflows/
    │       └── ci.yml
    └── .qt/
        ├── QtDeploySupport.cmake
        └── QtDeployTargets.cmake
```

# 📊 API Gateway Metrics & Monitoring

## Prometheus Metrics Endpoint

### Available Metrics

**Counters** – accumulate over time:

| Metric | Description | Labels |
|--------|------------|--------|
| `gateway_cache_hits_total` | Number of cache hits | `path` |
| `gateway_cache_misses_total` | Number of cache misses | `path` |
| `gateway_cache_store_total` | Number of times a response was stored in cache | `path` |
| `gateway_ratelimit_hits_total` | Requests blocked by rate limiter | `path`, `key` |
| `gateway_ratelimit_allowed_total` | Requests allowed by rate limiter | `path`, `key` |
| `gateway_backend_errors_total` | Backend requests that failed | `path` |
| `gateway_backend_timeout_total` | Backend request timeouts | `path` |
| `gateway_backend_status_total` | Backend status codes | `path`, `status` |
| `gateway_auth_ok_total` | Successful authentications | `path` |
| `gateway_auth_fail_total` | Failed authentications | `path` |
| `api_gateway_requests_total` | Total incoming requests | `path` |

**Gauges** – instantaneous values:

| Metric | Description |
|--------|------------|
| `gateway_active_clients` | Current number of connected clients |
| `gateway_active_requests` | Current number of in-flight requests |

**Histograms** – distribution of values:

| Metric | Description |
|--------|------------|
| `gateway_call_latency_seconds` | Backend call latency |
| `gateway_request_size_bytes` | Request sizes |
| `gateway_response_size_bytes` | Response sizes |
| `gateway_message_size_bytes` | Message sizes (incoming/outgoing) |

---

# Technologies

## Language & Standards
- **C++17** fallback for wider compatibility

## Build & Tooling
- **CMake + Ninja** 
- **clang-format / clang-tidy / cpplint** (formatting & static analysis)
- **vcpkg** 

## Networking & Web
- **Crow** (REST API backend)
- **WebSocket support** via Crow or `uWebSockets`

## Database & Caching
- **SQLite3** (lightweight embedded DB)
- **Redis++** (caching, rate limiting, pub/sub)

## Messaging / Queueing
- **RabbitMQ** (task queues, async messaging)

## Metrics & Monitoring
- **Prometheus-cpp** 
- **Grafana** 
- Metrics types:
  - **Counters**: requests, cache hits/misses, auth success/fail
  - **Gauges**: active clients, active requests
  - **Histograms**: latency, request size, response size, message size

## Logging & Tracing
- **spdlog** (structured, async logging)

## Testing & CI/CD
- **Catch2** (unit testing)
- **lcov + Codecov** (coverage reporting)
- **CI/CD**: GitHub Actions 

## Security / Auth
- JWT authentication (`JwtUtils`)

## Frontend / GUI
- **Qt6** (native desktop GUI)

## Links
- **C++ CI:**(https://github.com/roma3214185/messanger/actions/workflows/ci.yml/badge.svg)
- **Code Coverage (Codecov):** [https://app.codecov.io/github/roma3214185/messanger]

