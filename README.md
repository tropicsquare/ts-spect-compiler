# SPECT Compiler
This repository presents SPECT, a domain specific processor designed for
performing calculations related to Elliptic Curve Cryptography (ECC). SPECT
offers dedicated instructions for operations involving 256-bit numbers and
modular arithmetic, making it useful for implementing algorithms like ECDSA
(Elliptic Curve Digital Signature Algorithm) and ECDH (Elliptic Curve
Diffie-Hellman).
## Documentation [![build-docs](https://github.com/tropicsquare/ts-spect-compiler/actions/workflows/build-docs.yaml/badge.svg?branch=master)](https://github.com/tropicsquare/ts-spect-compiler/actions/workflows/build-docs.yaml)

### 1. [Programmer's Guide](doc/programmer_guide/pdf/spect_programmer_guide.pdf)

### 2. ISA (Instruction Set Architecture): 
- [v0.1](doc/ISAv0.1/pdf/isav0.1.pdf)
- [v0.2](doc/ISAv0.2/pdf/isav0.2.pdf)

## SPECT Compiler and Instruction Set Simulator

### Download Pre-built Binaries [![build-binaries](https://github.com/tropicsquare/ts-spect-compiler/actions/workflows/build-release-binaries.yaml/badge.svg)](https://github.com/tropicsquare/ts-spect-compiler/actions/workflows/build-release-binaries.yaml)

1. **Download the pre-built binaries:**
   
    Visit the [releases page](https://github.com/tropicsquare/ts-spect-compiler/releases)
    and download the binaries for the SPECT compiler and ISS (Instruction Set
    Simulator) that match your system architecture (either AMD64 or ARM64v7).
    The binaries are named in the format `spect_compiler_linux_<arch>_<git-tag>`
    and `spect_iss_linux_<arch>_<git-tag>`.

2. **Rename the Binaries**
   
    Rename the binaries to `spect_compiler` and `spect_iss`. These names are
    used by the Makefiles in the [`ts-spect-fw`](https://github.com/tropicsquare/ts-spect-fw) repository.

    Open a terminal, navigate to the directory containing the downloaded
    binaries, and rename them using the `mv` command.

    ```bash
    mv spect_compiler_linux_amd64_master spect_compiler
    mv spect_iss_linux_amd64_master spect_iss
    ```

2. **Make the Binaries Executable:**
   
    From the same directory containing the downloaded binaries, make them
    executable using the `chmod` command.

    ```bash
    chmod +x spect_compiler
    chmod +x spect_iss
    ```

3. **Run the Binaries:**

    Execute the binary with your desired command-line options.

    ```bash
    ./spect_compiler --help
    ./spect_iss --help
    ```

4. **Optional: Add to $PATH (Convenient Access):**
   For easy access from any directory, add the binary's directory to your
   system's `$PATH`. Edit your shell configuration file (e.g., `~/.bashrc`):

   ```bash
   # Replace '/path/to/directory' with the actual path
   echo 'export PATH="${PATH}:/path/to/directory"' >> ~/.bashrc  
   ```

   Restart your terminal or run `source ~/.bashrc`, and you can use `spect_compiler` and `spect_iss` from anywhere on your Linux system.

### Build from Source with Docker

If you prefer to build the SPECT compiler and instruction set simulator from
source using Docker, follow these steps:

1. **Clone the Repository:**

    First, make sure you have Git installed on your system and recursively clone
    this repository to your local machine:

    ```bash
    git clone https://github.com/tropicsquare/ts-spect-compiler.git --recurse-submodules

    cd ts-spect-compiler  # Navigate to the repository directory
    ```

2. **Build the Docker Image:**

    If Docker is not already installed on your system, you can follow the
    official Docker installation instructions for your system to get it set up:
    [Docker Installation Guide](https://docs.docker.com/get-docker/).

    Once docker is installed and configured on your system, build the Docker
    image from the root of the repository. This image will contain the necessary environment for building SPECT:

    ```bash
    docker build -t spect-env . -f Dockerfiles/Dockerfile
    ```

3. **Build the Binaries Inside the Docker Image:**

    Once the Docker image is built, you can use it to build the SPECT compiler. Run the following command to build the binaries inside the Docker container:

    ```bash
    docker run --rm -v $(pwd):/app -w /app spect-env ./build.sh --clean
    ```

    This command mounts your current working directory into the Docker container and executes the build process. The resulting binaries will be stored in `<repo_root>/build/src/apps`.

4. **Make the Binaries Executable:**

    If necessary, make the generated binaries executable as described in the previous instructions:

    ```bash
    chmod +x spect_compiler
    chmod +x spect_iss
    ```

5. **Optional: Add Binaries to $PATH:**

    To easily access these binaries from anywhere, add their directory to your system's `$PATH` as described in the previous instructions:

    ```bash
    export PATH="${PATH}:$(pwd)/build/src/apps/"
    ```

    You can also add this line to your shell configuration file (e.g., `~/.bashrc` or `~/.zshrc`) to make the changes persistent.

6. **Run the Binaries:**

    Finally, you can run the SPECT compiler and instruction set simulator:

    ```bash
    spect_compiler --help
    spect_iss --help
    ```

### Build from Source with CMake

Before you begin, ensure you have the following prerequisites installed on your system:

- <details>
    <summary>CMake (version 3.18.2 or higher)</summary>

    ```bash
    sudo apt-get install cmake
    ```
</details>

- <details>
    <summary>C++ compiler (like g++ or clang-*)</summary>

    ```bash
    sudo apt-get install g++
    ```
</details>

- <details>
    <summary>Python (version 3.8 or higher)</summary>

    ```bash
    sudo apt-get update
    sudo apt-get install python3-pip
    ```
</details>

- <details>
    <summary>Python packages (Jinja2)</summary>

    ```bash
    pip install jinja2
    ```
</details>

- <details>
    <summary>XsltProc</summary>

    ```bash
    sudo apt-get install xsltproc
    ```
</details>


Now, follow these steps to build SPECT from source with CMake:

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

We appreciate your interest in contributing to the SPECT compiler! Currently, we are not accepting pull requests. However, we are actively considering community contributions and may provide support for them in the future. Stay tuned for updates, and thank you for your understanding.
