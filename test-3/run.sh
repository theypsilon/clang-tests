#!/usr/bin/env bash

set -euo pipefail

./build/tool -shared -fPIC test.cpp -o build/libshared.so

