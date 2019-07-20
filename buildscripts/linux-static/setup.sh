#!/bin/bash

# Run inside a new, Debian-based container to set up a build environment the first time

set -e

apt-get install -y git build-essential python curl vim unzip libtool

# Qt
apt-get install -y "^libxcb.*" libx11-xcb-dev libglu1-mesa-dev libxrender-dev
apt-get install -y libfontconfig1-dev libglu1-mesa-dev libxrender-dev xkb-data
apt-get install -y libasound2-dev

# Tor
apt-get install -y autoconf automake

