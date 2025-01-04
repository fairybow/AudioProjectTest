#pragma once

#include <filesystem>
#include <vector>

class AudioAnalyzer
{
public:
    // Something like this
    struct Analysis
    {
        //std::vector<std::complex<double>> frequencyData; // Frequency-domain data (from FFT)
        //double samplingRate;                             // Sampling rate of the audio file
        //size_t fftSize;                                  // Size of the FFT
        //std::vector<double> timeDomainData;              // (Optional) Original time-domain samples
    };

    AudioAnalyzer();
    virtual ~AudioAnalyzer() = default;

    std::vector<Analysis> process(const std::vector<std::filesystem::path>& paths);
    Analysis process(const std::filesystem::path& path);

private:
    // m_plan;      // Cached plan for batch analysis, for speeeeeed
    // m_wisdom;    // read from disk

}; // class AudioAnalyzer
