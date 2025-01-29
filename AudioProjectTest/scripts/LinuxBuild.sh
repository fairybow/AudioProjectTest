#!/bin/bash

set -e  # Exit immediately if a command exits with a non-zero status

# Get the directory of the script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Move to the project root (one level above AudioProjectTest/scripts)
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Navigate to external directory
cd "$PROJECT_ROOT/AudioProjectTest/external"

# Extract FFTW
tar -xvf fftw-3.3.10.tar.gz
cd fftw-3.3.10

# Build & install FFTW
./configure --enable-static --disable-shared --enable-float
make -j$(nproc)
sudo make install

# Navigate back to the project root
cd "$PROJECT_ROOT"

# Create build directory and navigate into it
mkdir -p build && cd build

# Run CMake and build the project
cmake "$@" ..
make -j$(nproc)
