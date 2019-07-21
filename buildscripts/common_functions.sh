#!/bin/bash

# Describe the current version of the repo using git
git_version() {
  git describe --tags HEAD --always 2>/dev/null || git rev-parse --short HEAD
}
