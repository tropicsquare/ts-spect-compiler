#!/bin/bash

if [ "$1" == "--clean" ]; then
    echo "Deleting previous SPECT compiler build..."
    rm -rf build
fi
mkdir -p build

echo "*************************************************************************"
echo "* Generating System Verilog coverage class"
echo "*************************************************************************"

./generate_cover_class.py --input templates/InstructionDefs_v2.txt --output src/cosim/spect_instr_gen_coverage.svh --cov-template templates/coverage_class_template.txt --instr-defs templates/InstructionDefs_v2.txt


echo "*************************************************************************"
echo "* Initalizing SPECT compiler and model build"
echo "*************************************************************************"

cd build
cmake ..

echo "*************************************************************************"
echo "* Building SPECT compiler and model"
echo "*************************************************************************"

make
cd ..
