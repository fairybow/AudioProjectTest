/*#pragma once

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
        // Each analysis would, I guess, contain a note on each second of the
        // clip and whether that second is voice, static, etc.
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
    int fftSize_{};

    std::size_t numFrequencyBins_{};
    float* fftInputBuffer_ = nullptr;
    fftwf_complex* fftOutputBuffer_ = nullptr;
    fftwf_plan fftwPlan_ = nullptr;

    std::vector<float> m_hannWindow{};

    // m_plan;      // Cached plan for batch analysis, for speeeeeed
    // m_wisdom;    // read from disk

    void _initFftwPlan();
    void _freeFftwPlan();
    void _initHannWindow();
    std::vector<float> _toSamples(std::ifstream& rawAudio) const;
    std::streamsize sizeOf_(std::ifstream& stream) const;
    Analysis _analyzeFileSamples(const std::vector<float>& fileSamples) const;

}; // class AudioAnalyzer
*/
