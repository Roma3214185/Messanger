# ---------------- Build stage ----------------
FROM debian:bookworm AS builder

# Install build dependencies
RUN apt update && apt install -y \
    build-essential \
    cmake \
    git \
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-5compat-dev \
    libsqlite3-dev \
    libasio-dev \
    unzip \
    pkg-config \
    zlib1g-dev \
    libspdlog-dev \
    libfmt-dev \
    libhiredis-dev \
    libboost-chrono-dev \
    libboost-system-dev \
    libboost-thread-dev \
    librabbitmq-dev \
    nlohmann-json3-dev \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY CMakeLists.txt .
COPY common/ common/
COPY external/ external/
COPY Backend/ChatService Backend/ChatService/

ARG TARGET
RUN cmake -S . -B build -D TARGET=${TARGET} -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --target ${TARGET} --parallel

# ---------------- Runtime stage ----------------
FROM debian:bookworm-slim AS runtime

# Install only runtime dependencies
RUN apt update && apt install -y \
    libqt6core6 \
    libqt6widgets6 \
    libqt6sql6 \
    libqt6gui6 \
    libsqlite3-0 \
    libsodium23 \
    libssl3 \
    libcurl4 \
    librabbitmq4 \
    libboost-chrono1.74.0 \
    libboost-system1.74.0 \
    libspdlog1.10 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

ARG TARGET
COPY --from=builder /app/build/Backend/${TARGET}/${TARGET} /app/${TARGET}

EXPOSE 8081

ENTRYPOINT ["./ChatService"]
