#pragma once

#include "fftw3.h"

#include <cstddef>
#include <filesystem>
#include <vector>

// https://www.fftw.org/index.html
// We are using FFTW's single (float) precision, for speed.
// Note: implement batch analysis and be sure to reuse the fftwf_plan, for
// fastest speed, as per the FAQs.

// Do voice detection in-class, while processing each file, for speed. Can use a
// utility namespace.
// Can set it up, later, to use a function pointer / callback mechanism

class AudioAnalyzer
{
public:
    struct Analysis
    {
        bool containsVoice = false;             // True if the clip is likely to contain human voice
        float voiceConfidence = 0.0f;           // Confidence score (0.0 to 1.0)
        std::vector<float> spectralEnergy{};    // Energy in key bands
        //std::vector<float> formantPeaks;      // Detected formant frequencies
        // NOTE are formant peaks needed?
    };

    AudioAnalyzer();
    virtual ~AudioAnalyzer();

    std::vector<Analysis> process(const std::vector<std::filesystem::path>& inFiles, std::size_t fftSize = DEFAULT_FFT_SIZE);
    Analysis process(const std::filesystem::path& inFile, std::size_t fftSize = DEFAULT_FFT_SIZE);

private:
    static constexpr int DEFAULT_FFT_SIZE = 1024;
    static constexpr float m_pi = 3.141593f; // Accurate enough?
    // Make configurable?:
    static constexpr auto m_normalizationFactor = 1.0f / 32768.0f;
    static constexpr auto m_overlapPercentage = 0.5f; // 0.0 - 1.0

    // FFTW takes an int, but size_t as entry point (in process call) makes sense, I think
    int m_fftSize{};

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
