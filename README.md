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

rm -rf build
cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="--coverage -O0" \
    -DCMAKE_EXE_LINKER_FLAGS="--coverage" \
    -DCMAKE_SHARED_LINKER_FLAGS="--coverage" \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_TOOLCHAIN_FILE=$PWD/external/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_PREFIX_PATH="$HOME/local:$CMAKE_PREFIX_PATH" \
    -DTRACY_ENABLE=ON
cmake --build build --parallel
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

### 4. Running tests and generate coverage
All tests are integrated with CTest.
You can run them after building the project:
```bash
ctest --test-dir build --output-on-failure

lcov --directory build --capture --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/_deps/*' '*/include/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage-report

codecov -f coverage.info
```

## Project Structure
```bash
Directory structure:
└── roma3214185-messanger/
    ├── README.md
    ├── external/
    ├── Backend/
    │   ├── Gateway/
    │   │   ├── Dockerfile
    │   │   ├── include/
    │   │   ├── src/
    │   │   └── tests/
    │   ├── AuthService/
    │   │   ├── Dockerfile
    │   │   ├── include/
    │   │   ├── src/
    │   │   └── tests/
    │   ├── ChatService/
    │   │   ├── Dockerfile
    │   │   ├── include/
    │   │   ├── src/
    │   │   └── tests/
    │   ├── MessageService/
    │   │   ├── Dockerfile
    │   │   ├── include/
    │   │   ├── src/
    │   │   └── tests/
    │   ├── NotificationService/
    │   │   ├── Dockerfile
    │   │   ├── include/
    │   │   ├── src/
    │   │   └── tests/
    ├── common/
    │   ├── Constants/
    │   ├── Entities/
    │   ├── Metrics/
    │   ├── Network/
    │   ├── Persistence/
    │   │   ├── benchmarks/
    │   │   ├── include/
    │   │   ├── inl/
    │   │   ├── src/
    │   │   └── tests/
    │   ├── RabbitMQClient/
    │   └── RedisCache/
    ├── Frontend/
    │   ├── config/
    │   ├── forms/
    │   ├── include/
    │   ├── src/
    │   └── tests/
    ├── .github/
    │   └── workflows/
    │       └── ci.yml

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

## Persistence & GenericRepository Benchmarks

The `Persistence` module and `GenericRepository` have been benchmarked to measure performance improvements using **caching, Redis pipelines, and optimized entity building**. Key takeaways:

- **Query & Entity Cache:**  
  - Query caching gives up to **6× speedup**.  
  - Entity caching reduces repeated database access by **~4×**.  
- **Redis Pipeline:**  
  - Bulk saving 1000 entities is **~2× faster** with pipeline vs individual `SET`.  
- **Entity Builders:**  
  - Generic tuple-based builders are **~3–4× faster** than dynamic.  
- **Async / Thread Pool:**  
  - Useful for expensive queries, but adds minor overhead for cached operations.  
  
Benchmark scripts and detailed results can be found in:  
```bache
common/Persistence/benchmarks/
```
These optimizations ensure **high-performance, thread-safe, and low-latency data access** in the messenger backend.

# Technologies

## Language & Standards
- **C++17** fallback for wider compatibility

## Build & Tooling
- **CMake + Ninja** 
- **clang-format / clang-tidy / cpplint** (formatting & static analysis)
- **vcpkg** 

## Networking & Web
- **Crow** (REST API backend)
- **WebSocket support** via Crow or `IXWebSocket`

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

## Docker Support
- All backend microservices can be built and run as Docker containers.

## Benchmarks
- The `Persistence` module have been benchmarked to measure performance improvements 

## Testing & CI/CD
- **Unit tests:** Catch2  
- **Code coverage:** lcov + Codecov  
- **CI/CD:** GitHub Actions 

📊 Current coverage: **~74%**

## Security / Auth
- JWT authentication with public and private keys

## Frontend / GUI
- **Qt6** (native desktop GUI)

## Links
- **C++ CI:**(https://github.com/roma3214185/messanger/actions/workflows/ci.yml/badge.svg)
- **Code Coverage (Codecov):** [https://app.codecov.io/github/roma3214185/messanger]

