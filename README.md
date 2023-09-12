# SPECT Compiler
This repository presents SPECT, a domain specific processor designed for
performing calculations related to Elliptic Curve Cryptography (ECC). SPECT
offers dedicated instructions for operations involving 256-bit numbers and
modular arithmetic, making it useful for implementing algorithms like ECDSA
(Elliptic Curve Digital Signature Algorithm) and ECDH (Elliptic Curve
Diffie-Hellman).
## Documentation [![build-docs](https://github.com/tropicsquare/ts-spect-compiler/actions/workflows/build-docs.yaml/badge.svg?branch=master)](https://github.com/tropicsquare/ts-spect-compiler/actions/workflows/build-docs.yaml)

### 1. Programmer's Guide: [Internal](https://tropic-gitlab.corp.sldev.cz/internal/sw-design/ts-spect-compiler/-/jobs/artifacts/master/raw/public/spect_programmer_guide.pdf?job=pages) | [GitHub](doc/programmer_guide/pdf/spect_programmer_guide.pdf)

### 2. ISA (Instruction Set Architecture): 
- v0.1: [Internal](https://tropic-gitlab.corp.sldev.cz/internal/sw-design/ts-spect-compiler/-/jobs/artifacts/master/raw/public/isav0.1.pdf?job=pages) | [GitHub](doc/ISAv0.1/pdf/isav0.1.pdf)
- v0.2: [Internal](https://tropic-gitlab.corp.sldev.cz/internal/sw-design/ts-spect-compiler/-/jobs/artifacts/master/raw/public/isav0.2.pdf?job=pages) | [GitHub](doc/ISAv0.2/pdf/isav0.2.pdf)

## Build Compiler and Instruction Set Simulator

### Prerequisites

Before you begin, ensure you have the following prerequisites installed on your
system:

- <details>
    <summary>C++ compiler (like g++ or clang-*)</summary>

    ```bash
    sudo apt-get install g++
    ```
</details>

- <details>
    <summary>CMake (version 3.18.2 or higher)</summary>

    ```bash
    sudo apt-get install cmake
    ```
</details>

- <details>
    <summary>Python (version 3.8 or higher)</summary>

    ```bash
    sudo apt-get install python3-pip
    ```
</details>

- <details>
    <summary>Python packages (Jinja2)</summary>

    ```bash
    pip install jinja2
    ```
</details>

### Build with CMake
Run [`./build.sh --clean`](build.sh) from the repository root. This will
generate binaries for the compiler `spect_compiler` and the instruction set
simulator `spect_iss` in the default build directory `build/src/apps`.

### Add binaries to $PATH
```bash
export PATH="${PATH}:build/src/apps/"
```
You may also add this line to your shell configuration file (like `~/.bashrc`
or `~/.zshrc`) to access SPECT binaries within new sessions.

### Run
```bash
spect_compiler --help
spect_iss --help
```
