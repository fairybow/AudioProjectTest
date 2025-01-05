#pragma once

#include "fftw3.h"

#include <cstddef>
#include <filesystem>
#include <vector>

class AudioAnalyzer
{
public:
    // One second segments

    struct Analysis
    {
        // store segment size and provide conversion time function for vector index
        // vector could perhaps only store unit of segment index within the
        // audio clip and nothing else (so, we lose any semblance of the
        // original result vector, or we only ever collect the busted indexes)
    };

    AudioAnalyzer();
    virtual ~AudioAnalyzer();

    Analysis process(const std::filesystem::path& inFile);
    std::vector<Analysis> process(const std::vector<std::filesystem::path>& inFiles);

private:
    // int m_fftSize{};
    // ^ Hard code for now. Allow changing later.
    static constexpr int DEFAULT_FFT_SIZE = 1024;

    //...

}; // class AudioAnalyzer
