# Not made for Windows yet!

cmake_minimum_required(VERSION 3.20)
project(AudioProjectTest LANGUAGES CXX)

# C++ standard settings
set(CMAKE_CXX_STANDARD 20)
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source files
set(SOURCES
    src/AudioAnalyzer.cpp
    src/Diagnostics.cpp
    src/Logging.cpp
    src/Main.cpp
    src/Windowing.cpp
)

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES})

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
