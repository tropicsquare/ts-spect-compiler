# SPECT Compiler
This repository presents SPECT, a domain specific processor designed for
performing calculations related to Elliptic Curve Cryptography (ECC).
SPECT offers dedicated instructions for operations involving 256-bit numbers
and modular arithmetic, making it useful for implementing algorithms like:

- ECDSA (Elliptic Curve Digital Signature Algorithm)
- ECDH (Elliptic Curve Diffie-Hellman).
-
## Documentation

### [Programmer's Guide](doc/programmer_guide/pdf/spect_programmer_guide.pdf)

### ISA (Instruction Set Architecture):
- [v0.1](doc/ISAv0.1/pdf/isav0.1.pdf)
- [v0.2](doc/ISAv0.2/pdf/isav0.2.pdf) - Used in production TROPIC01 devices

## Dependencies

The `ts-spect-compiler` has following dependencies:

- cmake
- C++ compiler
- Python3.8 (or higher)
- Xlstproc
- jinja2 python package

To install the dependencies:

```
sudo apt-get install cmake build-essentials python3 python3-pip xsltproc
```

Then, to install `jinja2`:
```
pip install jinja2
```


### Build Instructions

Follow these steps to build SPECT from source with CMake:

1. **Clone the Repository:**

   First, recursively clone the SPECT repository to your local machine:

   ```bash
   git clone https://github.com/tropicsquare/ts-spect-compiler.git --recurse-submodules
   cd ts-spect-compiler  # Navigate to the repository directory
   ```

2. **Build SPECT:**

   Use the provided `build.sh` script to build SPECT with CMake:

   ```bash
   ./build.sh --clean
   ```

   This script will compile SPECT and generate binaries for the compiler `spect_compiler` and the instruction set simulator `spect_iss` in the default build directory, which is `build/src/apps`.

3. **Make the Binaries Executable (Optional):**

   If necessary, make the generated binaries executable as follows:

   ```bash
   chmod +x build/src/apps/spect_compiler
   chmod +x build/src/apps/spect_iss
   ```

4. **Optional: Add Binaries to $PATH (Convenient Access):**

   To easily access these binaries from anywhere, you can add their directory to your system's `$PATH`:

   ```bash
   export PATH="${PATH}:$(pwd)/build/src/apps/"
   ```

   You can also add this line to your shell configuration file (e.g., `~/.bashrc` or `~/.zshrc`) to make the changes persistent.

5. **Run the Binaries:**

   Finally, you can run the SPECT compiler and instruction set simulator using the following commands:

   ```bash
   spect_compiler --help
   spect_iss --help
   ```

## Contribution and Pull Requests

We appreciate your interest in contributing to the SPECT compiler! Currently, we are not accepting pull requests.
