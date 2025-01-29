#!/bin/bash

set -e  # Exit immediately if a command exits with a non-zero status

# Get the directory of the script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Repo strcture: SolutionDir/ProjectDir/subdir (like scripts)

# Move to the project root (one level above AudioProjectTest/scripts)
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Verify that CMakeLists.txt exists in PROJECT_ROOT
if [ ! -f "$PROJECT_DIR/CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found in $PROJECT_DIR"
    exit 1
fi

# Navigate to external directory
cd "$PROJECT_DIR/external"

# Extract FFTW
tar -xvf fftw-3.3.10.tar.gz
cd fftw-3.3.10

# Build & install FFTW
./configure --enable-static --disable-shared --enable-float
make -j$(nproc)
sudo make install

# Navigate back to the project root
cd "$PROJECT_DIR"

# Create build directory and navigate into it
mkdir -p build && cd build

# Run CMake and build the project
cmake "$@" "$PROJECT_DIR"
make -j$(nproc)
