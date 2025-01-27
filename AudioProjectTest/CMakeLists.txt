# Not made for Windows yet!

cmake_minimum_required(VERSION 3.20)
project(AudioProjectTest LANGUAGES CXX)

# C++ standard settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable AVX2 support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")

# Define build options
option(USE_AVX2 "Enable AVX2 support" OFF)
option(USE_DX_BENCH_MACROS "Enable DX Benchmark Macros" OFF)
option(USE_LOGGING "Enable Logging" OFF)

# Add the executable
set(SOURCES
    src/AudioAnalyzer.cpp
    src/Diagnostics.cpp
    src/Logging.cpp
    src/Main.cpp
    src/Windowing.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Conditionally define macros based on build options
if(USE_AVX2)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_AVX2)
endif()

if(USE_DX_BENCH_MACROS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_DX_BENCH_MACROS)
endif()

if(USE_LOGGING)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_LOGGING)
endif()

# Find FFTW (user can specify custom FFTW location)
find_path(FFTW_INCLUDE_DIR fftw3.h PATHS /usr/local/include)
find_library(FFTW_LIBRARY fftw3f PATHS /usr/local/lib)

# Ensure FFTW is found
if(NOT FFTW_INCLUDE_DIR OR NOT FFTW_LIBRARY)
    message(FATAL_ERROR "FFTW not found. Please install FFTW or specify the FFTW paths.")
endif()

# Include FFTW headers
target_include_directories(${PROJECT_NAME} PRIVATE ${FFTW_INCLUDE_DIR})

# Link FFTW library
target_link_libraries(${PROJECT_NAME} PRIVATE ${FFTW_LIBRARY})

# Optional: Diagnostics
message(STATUS "Using FFTW include dir: ${FFTW_INCLUDE_DIR}")
message(STATUS "Using FFTW library: ${FFTW_LIBRARY}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "USE_AVX2: ${USE_AVX2}")
message(STATUS "USE_DX_BENCH_MACROS: ${USE_DX_BENCH_MACROS}")
message(STATUS "USE_LOGGING: ${USE_LOGGING}")
