FROM ubuntu:18.04

RUN set -ex; \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        automake \
        autotools-dev \
        build-essential \
        curl \
        git \
        libevent-dev \
        libprotobuf-dev \
        libssl-dev \
        libtool \
        libxrender1 \
        pkg-config \
        protobuf-compiler \
        python \
        qt5-default \
        qt5-qmake \
        qtbase5-dev \
        qtdeclarative5-dev \
        qttools5-dev-tools \
        tor \
        unzip \
        vim \
        xvfb \
        ca-certificates \
        '^libxcb.*' \
        libx11-xcb-dev \
        libglu1-mesa-dev \
        libxrender-dev \
        libfontconfig1-dev \
        libglu1-mesa-dev \
        libxrender-dev \
        xkb-data \
        libasound2-dev \
        && rm -rf /var/lib/apt/lists/*

WORKDIR /root
RUN git clone https://github.com/blueprint-freespeech/ricochet-refresh
WORKDIR /root/ricochet-refresh/buildscripts/
RUN linux-static/build-deps.sh

