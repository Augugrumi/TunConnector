FROM alpine:latest

RUN apk update && \
    apk upgrade && \
    apk add cmake \
    build-base \
    wget \
    git \
    boost-dev \
    openvpn \
    nano && \
    rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/Augugrumi/TunConnector.git && \
    cd TunConnector && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make
