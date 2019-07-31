FROM ubuntu:18.04

RUN set -ex; \
    apt-get update \
    && apt-get install -y --no-install-recommends \
        automake \
        autotools-dev \
        build-essential \
        ca-certificates \
        curl \
        git \
        libasound2-dev \
        libevent-dev \
        libfontconfig1 \
        libfontconfig1-dev \
        libfreetype6 \
        libfreetype6-dev \
        libglu1-mesa-dev \
        libprotobuf-dev \
        libqt5gui5 \
        libqt5opengl5 \
        libqt5opengl5-dev \
        libqt5svg5 \
        libssl-dev \
        libtool \
        libx11-xcb-dev \
        '^libxcb.*' \
        '^libxkb.*' \
        libxrender-dev \
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
        xkb-data \
        xvfb \
    && apt-get autoremove -y -qq \
    && rm -rf /var/lib/apt/lists/* \
    && rm -rf /var/cache/man/* \
    && rm -rf /usr/share/doc/* \
    && rm -rf /usr/share/man/* \
    && rm -rf /usr/share/zoneinfo/*

WORKDIR /root
RUN git clone https://github.com/blueprint-freespeech/ricochet-refresh
WORKDIR /root/ricochet-refresh/buildscripts/
RUN linux-static/build-deps.sh

