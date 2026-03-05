# Messenger Application — Glossary

## Overview

This glossary defines the main technical terms, architectural concepts, and components used in the **Messenger Application**.  
The system is a **microservice-based distributed messaging platform** built with **C++ backend microservices and a Qt desktop frontend**.

The architecture includes:

- Microservices communicating via HTTP and events
- RabbitMQ for asynchronous communication
- Redis for caching
- SQLite with a custom ORM layer
- Crow framework for HTTP services
- Qt for the desktop client
- Docker containerization
- Prometheus & Grafana monitoring
- CI/CD pipeline for automated testing and deployment

---

# 1. Architecture Concepts

## Microservice

A **microservice** is an independent backend service responsible for a specific domain of the system.

Examples in this project:

- Auth Service – user authentication and registration
- Chat Service – chat creation and chat membership
- Message Service – message storage and queries
- Notification Service – real-time event distribution
- Gateway Service – API gateway for clients

Each microservice:

- runs independently
- has its own business logic
- communicates via HTTP or an event bus
- is containerized with Docker

---

## API Gateway

The **Gateway Service** acts as the **single entry point** for clients.

Responsibilities:

- routing requests to internal services
- authentication verification
- rate limiting
- caching
- metrics collection
- request forwarding

Key components:

- GatewayServer
- GatewayController
- ProxyClient
- Middleware chain

---

## Middleware

Middleware is a component that processes a request **before it reaches the main controller**.

Examples:

- AuthMiddleware – verifies JWT tokens
- RateLimitMiddleware – prevents request abuse
- MetricsMiddleware – collects metrics
- CacheMiddleware – checks Redis cache
- LoggingMiddleware – logs incoming requests

---

## Event-Driven Architecture

Services communicate asynchronously using **events** via RabbitMQ.

Typical flow:

Message saved → Event published → Notification service receives event → Users notified

Benefits:

- loose coupling between services
- scalable communication
- asynchronous processing

---

## Event Bus

The **Event Bus** is the messaging system used for service communication.

Implemented with:

RabbitMQ

Interfaces:

- IEventPublisher
- IEventSubscriber
- IEventBus

---

# 2. Backend Services

## Auth Service

Handles **user authentication and account management**.

Responsibilities:

- user registration
- login
- token generation
- user search by tag

Key components:

- Server
- AuthController
- AuthManager
- JwtGenerator
- RealAuthoritizer

---

## Chat Service

Manages chat entities and chat membership.

Responsibilities:

- create private chats
- manage chat members
- fetch chats of a user

Key classes:

- ChatServer
- ChatController
- ChatManager
- INetworkFacade

---

## Message Service

Handles all message-related data.

Responsibilities:

- saving messages
- deleting messages
- message queries
- reactions
- message status

Key components:

- Controller
- MessageCommandService
- MessageQueryService
- QueuePublisher
- QueueSubscriber

---

## Notification Service

Responsible for **real-time message delivery**.

Responsibilities:

- listening to message events
- delivering events via WebSocket
- notifying online users

Core components:

- SocketNotifier
- NotificationOrchestrator
- RabbitNotificationSubscriber
- RabbitNotificationPublisher

---

# 3. Frontend Architecture

The frontend is implemented as a **Qt desktop application** using the **Model–View–Presenter (MVP)** architecture.

## MVP Pattern

The application follows the **Model–View–Presenter (MVP)** pattern.

### Model

Responsible for:

- application state
- data storage
- domain models

Key classes:

- Model
- DataManager
- ChatModel
- UserModel
- MessageModel

---

### View

Responsible for UI rendering.

Main implementation:

MainWindow

Interface:

IMainWindow

Responsibilities:

- displaying chats
- showing messages
- handling user interaction

---

### Presenter

The **Presenter** connects the UI with business logic.

Responsibilities:

- handling user actions
- calling use cases
- updating the UI

Example actions:

- signIn()
- signUp()
- sendButtonClicked()
- onChatClicked()

---

# 4. Networking

## Network Manager

Handles HTTP communication between the frontend and backend services.

Interface:

INetworkAccessManager

Implementation:

NetworkAccessManager

Supports:

- GET
- POST
- PUT
- DELETE

---

## Network Facade

Provides a unified API for communication with backend services.

INetworkFacade

Sub-managers:

- IUserNetworkManager
- IMessageNetworkManager
- IChatNetworkManager

---

## Proxy Client

Used by services to **forward requests to other microservices**.

ProxyClient

---

# 5. Real-Time Communication

## WebSocket

Used for **real-time message delivery**.

Frontend component:

SocketManager

Backend components:

- CrowSocket
- SocketRepository
- SocketNotifier

---

## Socket Handler

Processes incoming WebSocket messages.

Interface:

IMessageHandler

Examples:

- SendMessageHandler
- DeleteMessageHandler
- MarkReadMessageHandler
- SaveMessageReactionHandler

---

## Socket Handler Registry

Registry responsible for mapping **message types to handlers**.

SocketHandlerRegistry

---

# 6. Authentication

## JWT (JSON Web Token)

Authentication tokens used for secure communication between client and services.

Generated by:

JwtGenerator

Verified by:

- RealAuthoritizer
- JWTVerifier

Utilities:

JwtUtils

---

## Token Manager

Frontend component responsible for storing the current session.

Responsibilities:

- storing authentication token
- storing current user ID
- resetting session

---

# 7. Data Layer

## Repository Pattern

The system uses the **Repository Pattern** for data persistence.

Main class:

template GenericRepository

Capabilities:

- save entity
- delete entity
- find by id
- query by field

---

## ORM Layer

Custom ORM abstraction over SQLite.

Components:

- SqlExecutor
- QueryFactory
- SelectQuery
- DeleteQuery
- SQLiteDatabase
- SQLiteQuery

---

## Cache Layer

Used to improve performance.

Backend cache:

RedisCache

Frontend cache:

RedisClient

Interfaces:

- ICache
- ICacheService

---

# 8. Asynchronous Processing

## Thread Pool

Used for executing background tasks.

Interface:

IThreadPool

Implementation:

ThreadPool

Typical usage:

- database queries
- event handling
- message processing

---

# 9. Event Processing

## Publisher

Publishes events to the event bus.

Example:

QueuePublisher

Events include:

- message saved
- message deleted
- reaction added
- reaction removed
- message status saved

---

## Subscriber

Consumes events from the event bus.

Examples:

- QueueSubscriber
- RabbitNotificationSubscriber

---

# 10. Data Models

## User

Represents a registered user.

Typical fields:

- user_id
- username
- tag
- password hash

---

## Chat

Represents a chat room.

Types:

- private chat
- group chat

Fields:

- chat_id
- members

---

## Message

Represents a message sent in a chat.

Fields:

- message_id
- chat_id
- sender_id
- content
- timestamp

---

## Reaction

Represents a reaction to a message.

Examples:

👍 ❤️ 😂 🔥

Fields:

- reaction_id
- message_id
- user_id

---

## Message Status

Represents the delivery state of a message.

Examples:

- sent
- delivered
- read

---

# 11. Observability

## Metrics

The system collects metrics for monitoring and performance analysis.

Examples:

- request latency
- service throughput
- error rates

Interface:

IMetrics

Implementation:

GatewayMetrics

---

## Monitoring Stack

Monitoring tools used in the system:

- Prometheus – metrics collection
- Grafana – visualization dashboards

---

# 12. DevOps Infrastructure

## Docker

All services are containerized using Docker.

Benefits:

- reproducible environments
- isolated services
- simplified deployment

---

## CI/CD Pipeline

Automated pipeline used for:

- building services
- running tests
- code quality checks
- generating documentation
- deploying containers

---

# 13. Qt UI Components

## Delegates

Qt item delegates responsible for rendering UI elements.

Examples:

- MessageDelegate
- ChatItemDelegate
- UserDelegate

---

## Theme System

Supports multiple application themes.

Interface:

ITheme

Implementations:

- LightTheme
- DarkTheme

---

# 14. Use Cases

Use cases represent **application-level business logic** and coordinate domain operations.

Examples:

- ChatUseCase
- MessageUseCase
- UserUseCase
- SessionUseCase
- SocketUseCase

They coordinate:

- managers
- repositories
- models
- networking components
