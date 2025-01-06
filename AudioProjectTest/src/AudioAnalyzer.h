#pragma once

#include "fftw3.h"

#include <cstddef>
#include <filesystem>
#include <vector>

class AudioAnalyzer
{
public:
    struct Analysis
    {
        std::filesystem::path file{};
        // FFT size determines the time resolution of static detection
        float chunkDurationSeconds = 0.0f;
        // Start times (in seconds) of detected static chunks
        std::vector<float> staticChunkStartTimes{};
    };

    AudioAnalyzer();
    virtual ~AudioAnalyzer();

    Analysis process(const std::filesystem::path& inFile);
    std::vector<Analysis> process(const std::vector<std::filesystem::path>& inFiles);

private:
    // FFT size represents the number of samples in a chunk
    // (2048 bytes with int16_t)
    static constexpr std::size_t DEFAULT_FFT_SIZE = 1024;
    static constexpr auto SAMPLING_RATE = 8000.0f;

    std::size_t m_fftSize = DEFAULT_FFT_SIZE;

    //--------------------------------------------------------------------------
    // Windowing
    //--------------------------------------------------------------------------

    static constexpr float PI = 3.141593f; // Accurate enough?
    std::vector<float> m_hannWindow = std::vector<float>(m_fftSize);
    // ^ Hard code for now. Allow changing later.

    void _initHannWindow();

    //--------------------------------------------------------------------------
    // FFTW
    //--------------------------------------------------------------------------

    // When/how to clear input/output buffers between (or not) transforms?

    std::size_t m_numFrequencyBins = 0;
    float* m_fftInputBuffer = nullptr;
    fftwf_complex* m_fftOutputBuffer = nullptr;
    fftwf_plan m_fftwPlan = nullptr;

    void _initFftw();
    void _freeFftw();

    //--------------------------------------------------------------------------
    // Processing
    //--------------------------------------------------------------------------

    void _processWithAvx2
    (
        std::vector<Analysis>& analyses,
        const std::vector<std::filesystem::path>& inFiles
    );

    void _processWithoutAvx2
    (
        std::vector<Analysis>& analyses,
        const std::vector<std::filesystem::path>& inFiles
    );

    void _fftAnalyzeChunk
    (
        const std::vector<int16_t>& chunk,
        float segmentStartTimeSeconds,
        std::vector<float>& staticChunkStartTimes
    );

}; // class AudioAnalyzer
