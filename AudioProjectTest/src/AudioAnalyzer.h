#pragma once

#include <filesystem>
#include <vector>

// https://www.fftw.org/index.html
// We are using FFTW's single (float) precision, for speed.
// Note: implement batch analysis and be sure to reuse the fftwf_plan, for
// fastest speed, as per the FAQs.

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