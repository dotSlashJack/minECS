# Base image with cross-compilation environment
FROM ubuntu:20.04 as builder
RUN apt-get clean && apt-get update
# Set non-interactive installation mode
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    git \
    build-essential \
    gcc-arm-linux-gnueabihf \
    g++-arm-linux-gnueabihf \
    cmake \
    wget \
    ca-certificates

# Setup the working directory
WORKDIR /build

# Clone WiringPi repository
RUN git clone https://github.com/WiringPi/WiringPi.git && \
    cd WiringPi && \
    ./build

# Copy your project files
COPY ./src /build/src
COPY ./include /build/include

# Compile the project
RUN arm-linux-gnueabihf-g++ -o projectName /build/src/main.cpp /build/src/GPIO.cpp -I/build/include -L/build/lib -lwiringPi

# Final image
FROM debian:buster-slim
COPY --from=builder /build/projectName /projectName

# Set entrypoint
ENTRYPOINT ["/projectName"]
