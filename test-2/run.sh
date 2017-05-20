#!/usr/bin/env bash

set -euo pipefail

LD_LIBRARY_PATH="$(pwd):${LD_LIBRARY_PATH:-}" ./main

