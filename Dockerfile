# 1. Build Stage
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install compiler, OpenSSL, libcurl, and nlohmann-json header packages
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libssl-dev \
    libcurl4-openssl-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy all source files
COPY . .

# Compile main.cpp and tideTest.cpp together with include flag -I.
RUN g++ -O3 -std=c++17 -I. main.cpp tideTest.cpp -o server -lpthread -lssl -lcrypto -lcurl

# 2. Runtime Stage
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

ENV TZ=America/Los_Angeles

# Install runtime libraries
RUN apt-get update && apt-get install -y \
    ca-certificates \
    libcurl4 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy compiled executable and cached conditions data
COPY --from=builder /app/server .
COPY --from=builder /app/conditions.json .

EXPOSE 8080

CMD ["./server"]