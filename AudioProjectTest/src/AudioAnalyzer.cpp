#define USE_AVX2 // Temp

#include "AudioAnalyzer.h"
#include "Diagnostics.h"

#include <iostream>

#if defined(USE_AVX2)
#pragma message("Using AVX2 SIMD.")

#include <immintrin.h>

/*static void convertToFloatAVX2(const std::int16_t* input, float* output, std::size_t count)
{
    std::size_t i = 0;
    for (; i + 15 < count; i += 16) {
        // Load 16 int16 samples into AVX2 register
        __m256i samples = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(input + i));

        // Convert lower 8 int16 samples to float
        __m256i lower = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(samples));
        __m256 floatLower = _mm256_cvtepi32_ps(lower);

        // Convert upper 8 int16 samples to float
        __m256i upper = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(samples, 1));
        __m256 floatUpper = _mm256_cvtepi32_ps(upper);

        // Store results
        _mm256_storeu_ps(output + i, floatLower);
        _mm256_storeu_ps(output + i + 8, floatUpper);
    }

    // Scalar processing for remaining samples
    for (; i < count; ++i) {
        output[i] = static_cast<float>(input[i]);
    }
}

static void zeroPadAVX2(float* data, std::size_t start, std::size_t end) {
    __m256 zero = _mm256_setzero_ps();
    std::size_t i = start;
    for (; i + 7 < end; i += 8) {
        _mm256_storeu_ps(data + i, zero);
    }

    // Scalar zero-padding for remaining elements
    for (; i < end; ++i) {
        data[i] = 0.0f;
    }
}

static void analyzeFFTMagnitudeAVX2(const fftwf_complex* fftOutput, float* magnitudes, std::size_t count) {
    std::size_t i = 0;
    for (; i + 7 < count; i += 8) {
        // Load 8 complex pairs (real and imaginary parts)
        __m256 real = _mm256_loadu_ps(reinterpret_cast<const float*>(fftOutput + i * 2));
        __m256 imag = _mm256_loadu_ps(reinterpret_cast<const float*>(fftOutput + i * 2 + 1));

        // Compute magnitude: sqrt(real^2 + imag^2)
        __m256 realSquared = _mm256_mul_ps(real, real);
        __m256 imagSquared = _mm256_mul_ps(imag, imag);
        __m256 magnitude = _mm256_sqrt_ps(_mm256_add_ps(realSquared, imagSquared));

        // Store result
        _mm256_storeu_ps(magnitudes + i, magnitude);
    }

    // Scalar processing for remaining elements
    for (; i < count; ++i) {
        float real = reinterpret_cast<const float*>(fftOutput)[i * 2];
        float imag = reinterpret_cast<const float*>(fftOutput)[i * 2 + 1];
        magnitudes[i] = std::sqrt(real * real + imag * imag);
    }
}*/

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

// Possibly avoid copying the inFiles vector and edit directly for avx2

std::vector<AudioAnalyzer::Analysis> AudioAnalyzer::process(const std::vector<std::filesystem::path>& inFiles)
{
    // Use the vector processing
    // https://www.fftw.org/fftw3_doc/SIMD-alignment-and-fftw_005fmalloc.html#SIMD-alignment-and-fftw_005fmalloc

#if defined(USE_AVX2)

    std::cout << "Processing files using AVX2..." << std::endl;

    // Aligned buffers for FFT input/output
    float* fftInput = reinterpret_cast<float*>(fftwf_malloc(m_fftSize * sizeof(float)));
    fftwf_complex* fftOutput = reinterpret_cast<fftwf_complex*>(fftwf_malloc((m_fftSize / 2 + 1) * sizeof(fftwf_complex)));

    if (!fftInput || !fftOutput) {
        throw std::runtime_error("Failed to allocate aligned FFT buffers.");
    }

    fftwf_plan fftPlan = fftwf_plan_dft_r2c_1d(m_fftSize, fftInput, fftOutput, FFTW_ESTIMATE);

    for (const auto& file : inFiles) {
        // Load file and split into chunks
        // Use AVX2 functions like convertToFloatAVX2 and zeroPadAVX2
        // Perform FFT and analyze results
    }

    fftwf_destroy_plan(fftPlan);
    fftwf_free(fftInput);
    fftwf_free(fftOutput);

#else // !defined(USE_AVX2)

    // Iterate through all inFiles and process one at a time

#endif // defined(USE_AVX2)

    return {}; // Temp
}
