#!/usr/bin/env bash

set -euo pipefail

clang -shared -fPIC shared.c -o libshared.so
clang -L$(pwd) main.c -o main -lshared
