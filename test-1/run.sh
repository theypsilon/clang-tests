#!/usr/bin/env bash

set -euo pipefail

./build/tool test.cpp test.cpp \
	-fsyntax-only \
	-I/usr/local/lib/clang/4.0.0/include/

