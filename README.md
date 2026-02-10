# Messenger App

This is a full-stack messenger application built with C++ backend microservices and a Qt frontend. It supports real-time messaging, asynchronous request handling, and high-throughput performance. The backend uses Crow framework, RabbitMQ, Redis, and a custom SQLite ORM. All services are containerized with Docker, monitored via Prometheus & Grafana, and integrated with a CI/CD pipeline for testing, code quality, and documentation.

## Table of Contents
- [Features](#features)
- [Installation & Build](#installation--build)
- [Project Architecture](#project-architecture)
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
 brew install cmake ninja autoconf autoconf-archive automake libtool ccache lcov clang-format rabbitmq-c
./external/vcpkg/bootstrap-vcpkg.sh

rm -rf build
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE=$PWD/external/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=arm64-osx \
  -Wno-dev
  
cmake --build build --parallel
```

## Project Architecture

The following diagrams illustrate the architecture of the **Messenger** project, showing the main services, their interactions, and internal structure.

### High-Level Overview
![Messenger System Overview](https://roma3214185.github.io/Messenger/architecture/system.svg)  
*A top-level view of all services and their interactions.*

### Service-Specific Architectures
- **Gateway Service**  
  ![Gateway Service](https://roma3214185.github.io/Messenger/architecture/services/gateway.svg)

- **Authentication Service (AuthService)**  
  ![Auth Service](https://roma3214185.github.io/Messenger/architecture/services/authservice.svg)

- **Chat Service**  
  ![Chat Service](https://roma3214185.github.io/Messenger/architecture/services/chatservice.svg)

- **Frontend**  
  ![Frontend](https://roma3214185.github.io/Messanger/architecture/services/frontend.svg)

- **Message Service**  
  ![Message Service](https://roma3214185.github.io/Messenger/architecture/services/messageservice.svg)

- **Notification Service**  
  ![Notification Service](https://roma3214185.github.io/Messenger/architecture/services/notificationservice.svg)

- **Persistence Layer**  
  ![Persistence Layer](https://roma3214185.github.io/Messenger/architecture/services/persistence.svg)


## 🐳 Docker Support & Persistence Benchmarks

### Docker Support
All backend microservices can be built and run as Docker containers. This simplifies deployment, ensures consistent environments, and isolates dependencies.  

```bash
docker-compose up --build
``` 

# Technologies

## Language & Standards
- **C++17** fallback for wider compatibility

## Build & Tooling
- **CMake + Ninja** 
- **clang-format / clang-tidy / cppcheck / includewhatyouuse / sanitizers / cpplint** (formatting & static analysis)
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
- Current coverage: **~74%**

## Security / Auth
- JWT authentication with public and private keys

## Frontend / GUI
- **Qt6** (native desktop GUI)

## Documentation
- **PlantUML**
- **OpenAPI**

## Links

- **C++ CI:** ![CI](https://github.com/roma3214185/Messenger/actions/workflows/ci.yml/badge.svg)  
- **Code Coverage (Codecov):** [View on Codecov](https://app.codecov.io/github/roma3214185/Messenger)  
- **Documentation (OpenAPI):** [View on GitHub Pages](https://roma3214185.github.io/Messenger/)
- **SonarCloud:** [View on Sonar Cloud](https://sonarcloud.io/summary/overall?id=Roma3214185_Messenger&branch=main)  
