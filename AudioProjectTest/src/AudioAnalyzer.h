#pragma once

#include "fftw3.h"

#include <cstddef>
#include <filesystem>
#include <vector>
#include <stdexcept>

class AudioAnalyzer
{
public:
    // One second segments

    struct Analysis
    {
        // Adjustable later
        // Will help give range for segment at x seconds
        float segmentSeconds = 1.0f;
        // Handle remainder somehow (range will be wrong if we go by starting point + segmentSize)

        std::vector<float> m_badSegmentStartingPoints{};
    };

    AudioAnalyzer();
    virtual ~AudioAnalyzer();

    Analysis process(const std::filesystem::path& inFile);
    std::vector<Analysis> process(const std::vector<std::filesystem::path>& inFiles);

private:
    static constexpr int DEFAULT_FFT_SIZE = 1024;
    static constexpr auto DEFAULT_SEGMENT_SECONDS = 1.0f;

    int m_fftSize = DEFAULT_FFT_SIZE;
    float m_segmentSeconds = DEFAULT_SEGMENT_SECONDS;
    // ^ Hard code for now. Allow changing later.

    //...

}; // class AudioAnalyzer
