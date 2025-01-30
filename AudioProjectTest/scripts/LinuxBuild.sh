#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e 

# Get the directory of the script
# Note: Repo strcture = SolutionDir/ProjectDir/subdir (src, scripts, etc)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
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

# Boolean build options (default OFF)
USE_AVX2=OFF
USE_DX_BENCH_MACROS=OFF
USE_LOGGING=OFF

# Process command-line arguments
# We're checking for:
# 1. Forced FFTW build
# 2. FFTW lib & header path custom args
CMAKE_ARGS=()
for arg in "$@"; do
    case $arg in
        --forcelibbuild)
            FORCE_FFTW=1
            ;;
        --avx2)
            USE_AVX2=ON
            ;;
        --dxbench)
            USE_DX_BENCH_MACROS=ON
            ;;
        --logging)
            USE_LOGGING=ON
            ;;
        --fftwlibpath=*)
            FFTW_LIBRARY_DIR="${arg#*=}"
            ;;
        --fftwincpath=*)
            FFTW_INCLUDE_DIR="${arg#*=}"
            ;;
        *)
            CMAKE_ARGS+=("$arg") # Preserve other args as CMake args
    esac
done

# Add boolean flags to CMake args
CMAKE_ARGS+=(
    "-DUSE_AVX2=$USE_AVX2"
    "-DUSE_DX_BENCH_MACROS=$USE_DX_BENCH_MACROS"
    "-DUSE_LOGGING=$USE_LOGGING"
    "-DFFTW_INCLUDE_DIR=$FFTW_INCLUDE_DIR"
    "-DFFTW_LIBRARY=$FFTW_LIBRARY_DIR/libfftw3f.a"
)

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

    # Verify that fftw-3.3.10.tar.gz exists in pwd
    if [ ! -f "fftw-3.3.10.tar.gz" ]; then
        echo "Error: FFTW archive not found in $PWD"
        exit 1
    fi

    # Extract FFTW
    tar -xvf fftw-3.3.10.tar.gz
    cd fftw-3.3.10

    # Build & install FFTW
    ./configure --enable-static --disable-shared --enable-float
    make -j$(nproc)
    sudo make install

    echo "FFTW built and installed."
fi

# Navigate back to the project root
cd "$PROJECT_DIR"

# Create build directory and navigate into it
mkdir -p build && cd build

# Run CMake and build the project
cmake "${CMAKE_ARGS[@]}" "$PROJECT_DIR"
make -j$(nproc)
