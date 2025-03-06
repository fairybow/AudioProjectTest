#pragma once

#include "Windowing.h"

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
    // FFT size represents the number of samples in a chunk (2048 bytes with
    // std::int16_t)
    static constexpr const std::size_t DEFAULT_FFT_SIZE = 1024;

    struct Analysis
    {
        std::filesystem::path file{};
        std::size_t fftSize = 0;
        Windowing::Window windowType = Windowing::None;
        float overlapDecPercent = 0.0f;
        // Error code (instead of throwing on bad files)?

        // FFT size determines the time resolution of static detection
        float chunkDurationSeconds = 0.0f;

        // Start times (in seconds) of detected static chunks
        std::vector<float> staticChunkStartTimes{};

        friend std::ostream& operator<<(std::ostream&, const Analysis&);
    };

    AudioAnalyzer
    (
        std::size_t fftSize = DEFAULT_FFT_SIZE,
        Windowing::Window windowType = DEFAULT_WINDOW,
        float overlap = DEFAULT_OVERLAP,
        const std::filesystem::path& wisdomPath = {}
    );

    explicit AudioAnalyzer(const std::filesystem::path& wisdomPath);
    virtual ~AudioAnalyzer();

    Analysis process(const std::filesystem::path& inFile);
    std::vector<Analysis> process(const std::vector<std::filesystem::path>& inFiles);

private:
    // Would it be fine to just use singleton and set benching on/off, or would
    // the added function calls (which aren't present when the macros are
    // no-ops) add up?
    //DX_BENCH(AudioAnalyzer); // Shut up, Intellisense

    std::size_t fftSize_;

    // Adjustable?
    static constexpr auto SAMPLING_RATE_ = 8000.0f;

    //--------------------------------------------------------------------------
    // Windowing
    //--------------------------------------------------------------------------

    // If the signal has abrupt changes at the boundaries of each chunk (e.g.,
    // due to no overlap), these discontinuities introduce high-frequency
    // artifacts, known as spectral leakage. So, I believe that without a window
    // we will always detect static in a chunk at its edges.

public:
    static constexpr auto DEFAULT_WINDOW = Windowing::Hann;
    static constexpr auto DEFAULT_OVERLAP = 0.5f;

private:
    float overlapDecPercent_;
    Windowing::Window windowType_;
    bool useWindowing_ = true;
    std::vector<float> window_{};

    void initWindow_();

    //--------------------------------------------------------------------------
    // FFTW
    //--------------------------------------------------------------------------

private:
    std::filesystem::path wisdomPath_;

    std::size_t numFrequencyBins_ = 0;
    float* fftInputBuffer_ = nullptr;
    fftwf_complex* fftOutputBuffer_ = nullptr;
    fftwf_plan fftwPlan_ = nullptr;

    void initFftw_();
    void freeFftw_();

    //--------------------------------------------------------------------------
    // Processing
    //--------------------------------------------------------------------------

private:
    enum class IsLastChunk_
    {
        No = 0,
        Yes
    };

    void process_
    (
        std::vector<Analysis>& analyses,
        const std::vector<std::filesystem::path>& inFiles
    );

    void fftAnalyzeChunk_
    (
        const std::vector<std::int16_t>& chunk,
        float segmentStartTimeSeconds,
        std::vector<float>& staticChunkStartTimes,
        IsLastChunk_ isLastChunk = {}
    );

    std::streamsize sizeOf_(std::ifstream& rawAudio) const;
    void prepareInputBuffer_(const std::vector<std::int16_t>& chunk);
    void zeroPadInputBuffer_(const std::vector<std::int16_t>& chunk);
    std::vector<float> magnitudesFromOutputBuffer_() const;
    bool haveStatic_(const std::vector<float>& magnitudes) const;

}; // class AudioAnalyzer
