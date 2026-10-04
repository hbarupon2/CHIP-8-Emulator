#!/bin/bash
set -e

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

if [ -n "$1" ]; then
    ./build/CHIP8 "$1"
fi
