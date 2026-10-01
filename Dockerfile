# 1. Build Stage
FROM ubuntu:22.04 AS builder

# Install build tools and dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    make \
    libssl-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source files
COPY . .

# Compile your server binary (adjust source filenames if needed)
RUN g++ -O3 -std=c++17 main.cpp tideTest.cpp -o server -lpthread -lssl -lcrypto -lcurl# 2. Runtime Stage
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    ca-certificates \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy binary and initial conditions cache file from builder
COPY --from=builder /app/server .
COPY --from=builder /app/conditions.json .

# Expose server port
EXPOSE 8080

# Run server
CMD ["./server"]