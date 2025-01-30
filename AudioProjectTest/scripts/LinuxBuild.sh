#!/bin/bash

set -e  # Exit immediately if a command exits with a non-zero status

# Get the directory of the script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Repo strcture: SolutionDir/ProjectDir/subdir (like scripts)

# Move to the project root (one level above AudioProjectTest/scripts)
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Default FFTW paths
FFTW_INCLUDE_DIR="/usr/local/include"
FFTW_LIBRARY_DIR="/usr/local/lib"

# Parse command-line arguments for custom FFTW paths
for arg in "$@"; do
    case $arg in
        -DFFTW_INCLUDE_DIR=*)
            FFTW_INCLUDE_DIR="${arg#*=}"
            ;;
        -DFFTW_LIBRARY=*)
            FFTW_LIBRARY_DIR="$(dirname "${arg#*=}")"
            ;;
    esac
done

# Verify that CMakeLists.txt exists in PROJECT_ROOT
if [ ! -f "$PROJECT_DIR/CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found in $PROJECT_DIR"
    exit 1
fi

# Check if FFTW is already installed
if [ -f "$FFTW_INCLUDE_DIR/fftw3.h" ] && \
   [ -f "$FFTW_LIBRARY_DIR/libfftw3f.a" ]; then
    echo "FFTW is already installed at:"
    echo "  Include: $FFTW_INCLUDE_DIR"
    echo "  Library: $FFTW_LIBRARY_DIR"
    echo "Skipping FFTW build..."
else
    echo "FFTW not found. Building FFTW..."
    
    # Navigate to external directory
    cd "$PROJECT_DIR/external"

    # Extract FFTW
    tar -xvf fftw-3.3.10.tar.gz
    cd fftw-3.3.10

    # Build & install FFTW
    ./configure --enable-static --disable-shared --enable-float
    make -j$(nproc)
    sudo make install
fi

# Navigate back to the project root
cd "$PROJECT_DIR"

# Create build directory and navigate into it
mkdir -p build && cd build

# Run CMake and build the project
cmake "$@" "$PROJECT_DIR"
make -j$(nproc)
