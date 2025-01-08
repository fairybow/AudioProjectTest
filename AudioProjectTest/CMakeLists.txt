cmake_minimum_required(VERSION 3.20)
project(AudioProjectTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    src/AudioAnalyzer.cpp
    src/Diagnostics.cpp
    src/Logging.cpp
    src/Main.cpp
    src/Windowing.cpp
)

set(HEADERS
    src/AudioAnalyzer.h
    src/Diagnostics.h
    src/Logging.h
    src/Windowing.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/FFTW
)

# Linker settings
target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/FFTW
)

# Link the FFTW library (I don't know if this works)
target_link_libraries(${PROJECT_NAME} PRIVATE
    fftw3f
)
