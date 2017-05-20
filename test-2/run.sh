#!/usr/bin/env bash

set -euo pipefail

LD_LIBRARY_PATH="$(pwd)/build:${LD_LIBRARY_PATH:-}" ./build/main

