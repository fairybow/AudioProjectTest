#pragma once

#define USE_DX_BENCH_MACROS // Temp
#define USE_AVX2 // Temp

#include "Diagnostics.h"

#include "fftw3.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ostream>
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

        friend std::ostream& operator<<(std::ostream&, const Analysis&);
    };

    AudioAnalyzer();
    virtual ~AudioAnalyzer();

    Analysis process(const std::filesystem::path& inFile);
    std::vector<Analysis> process(const std::vector<std::filesystem::path>& inFiles);

private:
    // Would it be fine to just use singleton and set benching on/off, or would
    // the added function calls (which aren't present when the macros are
    // no-ops) add up?
    //DX_BENCH(AudioAnalyzer); // Shut up, Intellisense

    // FFT size represents the number of samples in a chunk
    // (2048 bytes with std::int16_t)
    static constexpr const std::size_t DEFAULT_FFT_SIZE = 1024;
    static constexpr auto SAMPLING_RATE = 8000.0f;

    std::size_t m_fftSize = DEFAULT_FFT_SIZE;

    //--------------------------------------------------------------------------
    // Windowing
    //--------------------------------------------------------------------------

    // If the signal has abrupt changes at the boundaries of each chunk (e.g.,
    // due to no overlap), these discontinuities introduce high-frequency
    // artifacts, known as spectral leakage. So, I believe that without a window
    // we will always detect static in a chunk at its edges.

    std::vector<float> m_window = std::vector<float>(m_fftSize);
    // ^ Hard code for now. Allow changing later.

    void _initWindow();

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

    void _process
    (
        std::vector<Analysis>& analyses,
        const std::vector<std::filesystem::path>& inFiles
    );

    void _fftAnalyzeChunk
    (
        const std::vector<std::int16_t>& chunk,
        float segmentStartTimeSeconds,
        std::vector<float>& staticChunkStartTimes
    );

    std::streamsize _sizeOf(std::ifstream& rawAudio) const;
    void _prepareInputBuffer(const std::vector<std::int16_t>& chunk);
    void _maybeZeroPadInputBuffer(const std::vector<std::int16_t>& chunk);
    std::vector<float> _magnitudesFromOutputBuffer();
    bool _haveStatic(const std::vector<float>& magnitudes) const;

}; // class AudioAnalyzer
