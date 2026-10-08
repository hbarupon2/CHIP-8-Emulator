#!/bin/bash
set -e

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

if [ -n "$1" ]; then
    ./build/ASM8 "$1"
fi
