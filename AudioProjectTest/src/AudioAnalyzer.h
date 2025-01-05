#pragma once

#include "fftw3.h"

#include <cstddef>
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
        //std::vector<std::complex<float>> frequencyData;  // FFT results (complex spectrum)
        //std::vector<float> magnitudeSpectrum;           // Magnitude of the FFT results
        //double samplingRate;                             // Sampling rate of the audio
        std::size_t fftSize;                             // Size of the FFT used
        //std::vector<float> timeDomainData;              // Optional: original samples (useful for debugging)
        //std::vector<float> voicedRegions;               // Optional: flags/indicators of voiced segments
    };

    AudioAnalyzer();
    virtual ~AudioAnalyzer();

    std::vector<Analysis> process(const std::vector<std::filesystem::path>& inFiles, std::size_t fftSize = DEFAULT_FFT_SIZE);
    Analysis process(const std::filesystem::path& inFile, std::size_t fftSize = DEFAULT_FFT_SIZE);

private:
    static constexpr std::size_t DEFAULT_FFT_SIZE = 1024;
    static constexpr float m_pi = 3.141593; // Accurate enough?
    // Make configurable?:
    static constexpr auto m_normalizationFactor = 1.0f / 32768.0f;
    static constexpr auto m_overlapPercentage = 0.5f; // 0.0 - 1.0

    std::size_t m_fftSize{};

    std::size_t m_numFrequencyBins{};
    float* m_fftInputBuffer = nullptr;
    fftwf_complex* m_fftOutputBuffer = nullptr;
    fftwf_plan m_fftwPlan = nullptr;

    std::vector<float> m_hannWindow{};

    // m_plan;      // Cached plan for batch analysis, for speeeeeed
    // m_wisdom;    // read from disk

    void _initFftwPlan();
    void _freeFftwPlan();
    void _initHannWindow();
    std::vector<float> _toSamples(std::ifstream& rawAudio) const;
    std::streamsize _sizeOf(std::ifstream& stream) const;
    Analysis _analyzeFileSamples(const std::vector<float>& fileSamples) const;

}; // class AudioAnalyzer
