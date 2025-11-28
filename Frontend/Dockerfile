## Stage 1: Build
#FROM ubuntu:22.04 AS builder

## Install build dependencies
#RUN apt-get update && apt-get install -y \
#    build-essential \
#    cmake \
#    git \
#    qt6-base-dev \
#    qt6-tools-dev \
#    libssl-dev \
#    pkg-config \
#    curl \
#    && rm -rf /var/lib/apt/lists/*

#WORKDIR /build

## Copy project
#COPY . .

## Build Frontend
#RUN mkdir -p build && cd build && \
#    cmake .. -DCMAKE_BUILD_TYPE=Release && \
#    cmake --build . --target Frontend

## Stage 2: Runtime
#FROM ubuntu:22.04

## Install runtime libraries for Qt
#RUN apt-get update && apt-get install -y \
#    qt6-base \
#    qt6-wayland \
#    libssl1.1 \
#    libstdc++6 \
#    && rm -rf /var/lib/apt/lists/*

#WORKDIR /app

## Copy built frontend executable and config
#COPY --from=builder /build/build/Frontend ./Frontend
#COPY --from=builder /build/Frontend/config ./config

## Make executable
#RUN chmod +x ./Frontend

## If running headless, you can skip exposing ports; if GUI via VNC/X11, configure later

#ENTRYPOINT ["./Frontend"]
