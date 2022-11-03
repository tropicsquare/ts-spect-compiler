#!/bin/bash

if [ "$1" == "--clean" ]; then
    echo "Deleting previous SPECT compiler build..."
    rm -rf build
fi
mkdir -p build

echo "*************************************************************************"
echo "* Initalizing SPECT compiler and model build"
echo "*************************************************************************"

cd build
cmake ..

echo "*************************************************************************"
echo "* Building SPECT compiler and model"
echo "*************************************************************************"

make -j4
cd ..