//#define USE_AVX2 // Temp

#include "AudioAnalyzer.h"
#include "Diagnostics.h"

#include <iostream>

#if defined(USE_AVX2)
#pragma message("Using AVX2 SIMD.")
#include <immintrin.h>
#endif

AudioAnalyzer::AudioAnalyzer()
{
    // Load the wisdom file here?
}

AudioAnalyzer::~AudioAnalyzer()
{
    // Free FFTW stuff
}

// Convenience overload for single process
AudioAnalyzer::Analysis AudioAnalyzer::process(const std::filesystem::path& inFile)
{
    return process(std::vector<std::filesystem::path>{ inFile }).at(0);
}

std::vector<AudioAnalyzer::Analysis> AudioAnalyzer::process(const std::vector<std::filesystem::path>& inFiles)
{
    // Use the vector processing
    // https://www.fftw.org/fftw3_doc/SIMD-alignment-and-fftw_005fmalloc.html#SIMD-alignment-and-fftw_005fmalloc

#if defined(USE_AVX2)

    // Define the general path first

#else // !defined(USE_AVX2)

    //...

#endif // defined(USE_AVX2)

    return {}; // Temp
}
