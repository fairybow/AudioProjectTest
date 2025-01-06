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
        // Adjustable later
        // Will help give range for segment at x seconds
        float segmentSeconds = 0.0f;
        // Handle remainder somehow (range will be wrong if we go by starting point + segmentSize)

        std::vector<float> staticSegmentStarts{};
    };

    AudioAnalyzer();
    virtual ~AudioAnalyzer();

    Analysis process(const std::filesystem::path& inFile);
    std::vector<Analysis> process(const std::vector<std::filesystem::path>& inFiles);

private:
    static constexpr std::size_t DEFAULT_FFT_SIZE = 1024;
    static constexpr auto DEFAULT_SEGMENT_SECONDS = 1.0f;
    // ^ Definitely hard coded.

    std::size_t m_fftSize = DEFAULT_FFT_SIZE;
    float m_segmentSeconds = DEFAULT_SEGMENT_SECONDS;
    static constexpr auto SAMPLING_RATE = 8000.0f;

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
    fftwf_plan mop_fftwPlan = nullptr;

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

    std::streamsize _sizeOf(std::ifstream& stream) const;
    float _seconds(std::streamsize streamSize) const;

}; // class AudioAnalyzer
