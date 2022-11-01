#!/bin/bash

if [ "$1" == "--clean" ]; then
    echo "Deleting previous SPECT compiler build..."
    rm -rf build
fi
mkdir -p build

cd build
cmake ..
make
cd ..