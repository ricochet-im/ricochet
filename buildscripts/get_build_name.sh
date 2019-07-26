#!/bin/bash

source common_functions.sh

RICOCHET_VERSION="$(git_version)"
TOR_VERSION="$(cd src/tor && git_version)"
OPENSSL_VERSION="$(cd src/openssl && git_version)"
echo "ricochet-${RICOCHET_VERSION} tor-${TOR_VERSION} openssl-${OPENSSL_VERSION}"
