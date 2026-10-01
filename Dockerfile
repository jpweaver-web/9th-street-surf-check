# 1. Build Stage
FROM ubuntu:22.04 AS builder

# Prevent interactive prompts during apt installation
ENV DEBIAN_FRONTEND=noninteractive

# Install g++, make, OpenSSL headers, AND libcurl headers
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libssl-dev \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy project files
COPY . .

# Compile C++ backend
RUN g++ -O3 -std=c++17 main.cpp tideTest.cpp -o server -lpthread -lssl -lcrypto -lcurl

# 2. Runtime Stage
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install runtime dependencies for curl and SSL
RUN apt-get update && apt-get install -y \
    ca-certificates \
    libcurl4 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy binary and cached conditions JSON from builder
COPY --from=builder /app/server .
COPY --from=builder /app/conditions.json .

EXPOSE 8080

CMD ["./server"]