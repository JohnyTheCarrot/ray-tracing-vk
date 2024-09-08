#!/usr/bin/bash

mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug -G "Ninja" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_EXPORT_COMPILE_COMMANDS=1 .. && cmake --build . --target raytracing
