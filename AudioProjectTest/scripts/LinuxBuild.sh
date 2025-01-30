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

# Default FFTW paths
FFTW_INCLUDE_DIR="/usr/local/include"
FFTW_LIBRARY_DIR="/usr/local/lib"

# Flag to force FFTW rebuild
FORCE_FFTW=0

# Process command-line arguments
# We're checking for:
# 1. Forced FFTW build
# 2. FFTW lib & header path custom args
CMAKE_ARGS=()
for arg in "$@"; do
    case $arg in
        --force-fftw)
            FORCE_FFTW=1
            ;;
        -DFFTW_INCLUDE_DIR=*)
            FFTW_INCLUDE_DIR="${arg#*=}"
            CMAKE_ARGS+=("$arg")
            ;;
        -DFFTW_LIBRARY=*)
            FFTW_LIBRARY_DIR="$(dirname "${arg#*=}")"
            CMAKE_ARGS+=("$arg")
            ;;
        *)
            CMAKE_ARGS+=("$arg")  # Store all other arguments for CMake
            ;;
    esac
done

# Check if FFTW is already installed
if [ $FORCE_FFTW -eq 0 ] && [ -f "$FFTW_INCLUDE_DIR/fftw3.h" ] && \
   [ -f "$FFTW_LIBRARY_DIR/libfftw3f.a" ]; then
    echo "FFTW is already installed at:"
    echo "  Include: $FFTW_INCLUDE_DIR"
    echo "  Library: $FFTW_LIBRARY_DIR"
    echo "Skipping FFTW build..."
else
    echo "Building FFTW (forced rebuild: $FORCE_FFTW)..."

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
cmake "${CMAKE_ARGS[@]}" "$PROJECT_DIR"
make -j$(nproc)
