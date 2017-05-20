#!/usr/bin/env bash

set -euo pipefail

mkdir -p build

clang -shared -fPIC shared.c -o build/libshared.so
clang -L$(pwd)/build main.c -o build/main -lshared
