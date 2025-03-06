#include "AudioAnalyzer.h"
#include "Windowing.h"

#include "fftw3.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(USE_AVX2)

// AVX2 can accelerate operations involving repetitive mathematical computations
// on vectors.
#pragma message("Using AVX2 SIMD.")

#include <immintrin.h>

#endif

std::ostream& operator<<(std::ostream& os, const AudioAnalyzer::Analysis& a)
{
    std::ostringstream oss{};
    oss << "File: " << a.file.string() << "\n"
        << "FFT Size: " << a.fftSize << "\n"
        << "Windowing: " << Windowing::toString(a.windowType) << "\n"
        << "Overlap: " << (a.overlapDecPercent * 100.0f) << "%\n"
        << "Chunk length (seconds): " << std::fixed << std::setprecision(2) << a.chunkDurationSeconds << "\n"
        << "Staticky chunk start times: [";

    // Format staticChunkStartTimes as a comma-separated list
    for (std::size_t i = 0; i < a.staticChunkStartTimes.size(); ++i)
    {
        if (i > 0) oss << ", ";
        oss << std::fixed << std::setprecision(2) << a.staticChunkStartTimes[i];
    }
    oss << "]";

    return os << oss.str();
}

AudioAnalyzer::AudioAnalyzer
(
    std::size_t fftSize,
    Windowing::Window windowType,
    float overlap,
    const std::filesystem::path& wisdomPath
)
    : fftSize_(std::max(std::size_t(1), fftSize))
    , windowType_(windowType)
    , overlapDecPercent_(std::clamp(overlap, 0.0f, 0.9f))
    , wisdomPath_(wisdomPath)
{
    initFftw_();
    initWindow_();
}

AudioAnalyzer::AudioAnalyzer(const std::filesystem::path& wisdomPath)
    : AudioAnalyzer(DEFAULT_FFT_SIZE, DEFAULT_WINDOW, DEFAULT_OVERLAP, wisdomPath)
{
}

AudioAnalyzer::~AudioAnalyzer()
{
    freeFftw_();
}

// Convenience overload for single process
AudioAnalyzer::Analysis AudioAnalyzer::process(const std::filesystem::path& inFile)
{
    return process(std::vector<std::filesystem::path>{ inFile }).at(0);
}

std::vector<AudioAnalyzer::Analysis> AudioAnalyzer::process(const std::vector<std::filesystem::path>& inFiles)
{
    std::vector<Analysis> analyses(inFiles.size());
    process_(analyses, inFiles);
    return analyses;
}

// Section off wisdom read/write
void AudioAnalyzer::initFftw_()
{
    // https://www.fftw.org/doc/SIMD-alignment-and-fftw_005fmalloc.html
    // fftwf_alloc_real & fftwf_alloc_complex are wrappers that call
    // fftwf_malloc

    numFrequencyBins_ = (fftSize_ / 2) + 1;
    fftInputBuffer_ = fftwf_alloc_real(fftSize_);
    fftOutputBuffer_ = fftwf_alloc_complex(numFrequencyBins_);

    if (!fftInputBuffer_ || !fftOutputBuffer_)
    {
        throw std::runtime_error("Failed to allocate FFT buffers.");
    }

    auto fft_size = static_cast<int>(fftSize_);

    if (!wisdomPath_.empty())
    {
        // https://fftw.org/fftw3_doc/Words-of-Wisdom_002dSaving-Plans.html

        // Try to load wisdom from file
        auto wisdom_path_str = wisdomPath_.string();
        auto wisdom_found = static_cast<bool>(
            fftwf_import_wisdom_from_filename(wisdom_path_str.c_str()));

        if (wisdom_found)
        {
            std::cout << "Wisdom file loaded from " << wisdom_path_str << std::endl;
        }
        else
        {
            std::cerr << "Failed to find wisdom file at " << wisdom_path_str << std::endl;
        }

        fftwPlan_ = fftwf_plan_dft_r2c_1d
        (
            fft_size,
            fftInputBuffer_,
            fftOutputBuffer_,
            FFTW_MEASURE
        );

        if (!std::filesystem::exists(wisdomPath_.parent_path()))
        {
            std::filesystem::create_directories(wisdomPath_.parent_path());
        }

        // Double check that this saves appropriately with variable FFT size,
        // overlap, etc. It should save only when an optimzed plan for the
        // current values wasn't found.
        if (!wisdom_found)
        {
            if (fftwf_export_wisdom_to_filename(wisdom_path_str.c_str()))
            {
                std::cout << "Wisdom file saved to " << wisdom_path_str << std::endl;
            }
            else
            {
                std::cerr << "Failed to save wisdom to disk (" << wisdom_path_str << ")" << std::endl;
            }
        }
    }
    else // (wisdomPath_.empty())
    {
        fftwPlan_ = fftwf_plan_dft_r2c_1d
        (
            fft_size,
            fftInputBuffer_,
            fftOutputBuffer_,
            FFTW_ESTIMATE
        );
    }
}

void AudioAnalyzer::freeFftw_()
{
    // If we only call this in destructor, then I guess we don't need checks or
    // nullptr assignment
    /*if (fftwPlan_)
    {
        fftwf_destroy_plan(fftwPlan_);
        fftwPlan_ = nullptr;
    }

    if (fftOutputBuffer_)
    {
        fftwf_free(fftOutputBuffer_);
        fftOutputBuffer_ = nullptr;
    }

    if (fftInputBuffer_)
    {
        fftwf_free(fftInputBuffer_);
        fftInputBuffer_ = nullptr;
    }*/

    fftwf_destroy_plan(fftwPlan_);
    fftwf_free(fftOutputBuffer_);
    fftwf_free(fftInputBuffer_);
}

void AudioAnalyzer::initWindow_()
{
    // Can perhaps get some benefit from testing different window types.
    // Ultimately, may only need one.
    switch (windowType_)
    {
    case Windowing::Triangular:
        window_ = Windowing::triangular(fftSize_);
        break;
    case Windowing::Hann:
        window_ = Windowing::hann(fftSize_);
        break;
    case Windowing::Hamming:
        window_ = Windowing::hamming(fftSize_);
        break;
    case Windowing::Blackman:
        window_ = Windowing::blackman(fftSize_);
        break;
    case Windowing::FlatTop:
        window_ = Windowing::flatTop(fftSize_);
        break;
    case Windowing::Gaussian:
        window_ = Windowing::gaussian(fftSize_);
        break;

    default:
    case Windowing::None:
        useWindowing_ = false;
        break;
    }
}

// Parallelize FFT Computation?
// Find common elements later
// Refine into smaller functions (potentially called from both this and AVX2
// function)
void AudioAnalyzer::process_
(
    std::vector<Analysis>& analyses,
    const std::vector<std::filesystem::path>& inFiles
)
{
    if (inFiles.empty())
    {
        throw std::invalid_argument("No input files provided.");
    }

    for (std::size_t i = 0; i < inFiles.size(); ++i)
    {
        std::vector<float> static_chunk_start_times{}; // Eventual product
        auto& in_file = inFiles[i];

        // Should we throw or just continue (and add an error enum to result
        // Analysis for this file, or something)
        if (!std::filesystem::exists(in_file))
        {
            std::ostringstream oss{};
            oss << "\"" << in_file.string() << "\" does not exist.";
            throw std::runtime_error(oss.str());
        }

        if (!std::filesystem::is_regular_file(in_file))
        {
            std::ostringstream oss{};
            oss << "\"" << in_file.string() << "\" is not a regular file.";
            throw std::runtime_error(oss.str());
        }

        std::ifstream raw_audio(in_file, std::ios::binary);

        if (!raw_audio)
        {
            std::ostringstream oss{};
            oss << "Unable to open file at \"" << in_file.string() << "\"";
            throw std::runtime_error(oss.str());
        }

        // Calculate raw audio stream size
        auto raw_audio_size = sizeOf_(raw_audio);

        // Calculate number of chunks
        // Before separating this off, we will need other variables in it
        // (chunks_count, for example)...
        auto hop_size = static_cast<std::size_t>(fftSize_ * (1.0f - overlapDecPercent_));
        auto total_samples = raw_audio_size / sizeof(std::int16_t);

        // Handle edge case where total_samples < fftSize_
        auto chunks_count = (total_samples > fftSize_)
            ? (((total_samples - fftSize_) / hop_size) + 1)
            : 1;

        if (chunks_count < 1)
        {
            throw std::runtime_error("Lol what");
        }

        std::vector<std::int16_t> buffer(fftSize_);

        if (chunks_count == 1)
        {
            raw_audio.read
            (
                reinterpret_cast<char*>(buffer.data()),
                total_samples * sizeof(std::int16_t)
            );

            fftAnalyzeChunk_
            (
                buffer,
                0.0f,
                static_chunk_start_times,
                IsLastChunk_::Yes
            );
        }
        else // (chunks_count > 1)
        {
            // Determine if there's a remainder based on the hop_size
            auto has_remainder = (total_samples > (chunks_count * hop_size));

            // Analyze full chunks and record start time (in seconds) of chunks with static
            for (std::size_t chunk_i = 0; chunk_i < chunks_count; ++chunk_i)
            {
                raw_audio.read
                (
                    reinterpret_cast<char*>(buffer.data()),
                    fftSize_ * sizeof(std::int16_t)
                );

                auto chunk_start_time = static_cast<float>(chunk_i * hop_size) / SAMPLING_RATE_;

                fftAnalyzeChunk_
                (
                    buffer,
                    chunk_start_time,
                    static_chunk_start_times
                );

                // Seek back to account for overlap
                // Sliding buffer instead?
                raw_audio.seekg
                (
                    -static_cast<std::streamoff>(fftSize_ - hop_size) * sizeof(std::int16_t),
                    std::ios::cur
                );
            }

            // Analyze remainder
            if (has_remainder)
            {
                std::size_t remainder_samples = total_samples - (chunks_count * hop_size);
                std::vector<std::int16_t> remainder_buffer(fftSize_, 0);

                raw_audio.read
                (
                    reinterpret_cast<char*>(remainder_buffer.data()),
                    remainder_samples * sizeof(std::int16_t)
                );

                auto remainder_start_time = static_cast<float>(chunks_count * hop_size) / SAMPLING_RATE_;

                fftAnalyzeChunk_
                (
                    remainder_buffer,
                    remainder_start_time,
                    static_chunk_start_times,
                    IsLastChunk_::Yes
                );
            }
        }

        // Aggregate results
        analyses[i] =
        {
            in_file,
            fftSize_,
            windowType_,
            overlapDecPercent_,
            static_cast<float>(fftSize_) / SAMPLING_RATE_,
            static_chunk_start_times
        };
    }
}

void AudioAnalyzer::fftAnalyzeChunk_
(
    const std::vector<std::int16_t>& chunk,
    float segmentStartTimeSeconds,
    std::vector<float>& staticChunkStartTimes,
    IsLastChunk_ isLastChunk
)
{
    prepareInputBuffer_(chunk);

    if (isLastChunk == IsLastChunk_::Yes)
    {
        zeroPadInputBuffer_(chunk);
    }

    fftwf_execute(fftwPlan_);

    auto magnitudes = magnitudesFromOutputBuffer_();

    if (haveStatic_(magnitudes))
    {
        staticChunkStartTimes.emplace_back(segmentStartTimeSeconds);
    }
}

std::streamsize AudioAnalyzer::sizeOf_(std::ifstream& rawAudio) const
{
    rawAudio.seekg(0, std::ios::end);
    std::streamsize raw_audio_size = rawAudio.tellg();
    rawAudio.seekg(0, std::ios::beg);
    return raw_audio_size;
}

// Copy chunk data into FFT input buffer with scaling and Hann window
// Add optional windows and an option for none
void AudioAnalyzer::prepareInputBuffer_(const std::vector<std::int16_t>& chunk)
{

#if !defined(USE_AVX2)

    // Checking outside the for loop faster? Negligible, I assume?
    if (useWindowing_)
    {
        for (std::size_t i = 0; i < chunk.size(); ++i)
            fftInputBuffer_[i] = chunk[i] * window_[i];
    }
    else
    {
        for (std::size_t i = 0; i < chunk.size(); ++i)
            fftInputBuffer_[i] = chunk[i];
    }

#else // defined(USE_AVX2)

    const auto chunk_size = chunk.size();
    std::size_t i = 0;

    if (useWindowing_)
    {
        // Process 8 elements at a time
        for (; i + 7 < chunk_size; i += 8)
        {
            // Load 8 int16_t values and extend to int32_t
            auto chunk_vals_16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&chunk[i]));
            auto chunk_vals = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(chunk_vals_16));

            // Load Hann window coefficients
            auto window_vals = _mm256_loadu_ps(&window_[i]);

            // Perform element-wise multiplication
            auto result = _mm256_mul_ps(chunk_vals, window_vals);

            // Store the results
            _mm256_storeu_ps(&fftInputBuffer_[i], result);
        }
    }
    else // (!useWindowing_)
    {
        for (; i + 7 < chunk_size; i += 8)
        {
            auto chunk_vals_16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&chunk[i]));
            auto chunk_vals = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(chunk_vals_16));

            // Store the results directly
            _mm256_storeu_ps(&fftInputBuffer_[i], chunk_vals);
        }
    }

    // Process remaining elements
    if (useWindowing_)
    {
        for (; i < chunk_size; ++i)
            fftInputBuffer_[i] = chunk[i] * window_[i];
    }
    else
    {
        for (; i < chunk_size; ++i)
            fftInputBuffer_[i] = chunk[i];
    }

#endif // !defined(USE_AVX2)

}

// Zero-pad the remainder of the buffer if the chunk is smaller than
// fftSize_ (which I would assume is almost always the case)
void AudioAnalyzer::zeroPadInputBuffer_(const std::vector<std::int16_t>& chunk)
{

#if !defined(USE_AVX2)

    std::fill
    (
        fftInputBuffer_ + chunk.size(),
        fftInputBuffer_ + fftSize_,
        0.0f
    );

#else // defined(USE_AVX2)

    auto zero_vec = _mm256_setzero_ps();
    std::size_t j = chunk.size();
    for (; j + 7 < fftSize_; j += 8)
    {
        _mm256_storeu_ps(&fftInputBuffer_[j], zero_vec);
    }

    // Process remaining elements
    for (; j < fftSize_; ++j)
    {
        fftInputBuffer_[j] = 0.0f;
    }

#endif // !defined(USE_AVX2)

}

// Analyze FFT output (magnitude calculation for each frequency bin)
std::vector<float> AudioAnalyzer::magnitudesFromOutputBuffer_() const
{
    std::vector<float> magnitudes(numFrequencyBins_);

#if !defined(USE_AVX2)

    for (std::size_t k = 0; k < numFrequencyBins_; ++k)
    {
        auto real = fftOutputBuffer_[k][0];
        auto imag = fftOutputBuffer_[k][1];
        magnitudes[k] = std::sqrt((real * real) + (imag * imag));
    }

#else // defined(USE_AVX2)

    std::size_t k = 0;
    for (; k + 7 < numFrequencyBins_; k += 8)
    {
        auto real_vals = _mm256_loadu_ps(&fftOutputBuffer_[k][0]);
        auto imag_vals = _mm256_loadu_ps(&fftOutputBuffer_[k][1]);

        auto real_sq = _mm256_mul_ps(real_vals, real_vals);
        auto imag_sq = _mm256_mul_ps(imag_vals, imag_vals);

        auto magnitude = _mm256_sqrt_ps(_mm256_add_ps(real_sq, imag_sq));
        _mm256_storeu_ps(&magnitudes[k], magnitude);
    }

    // Process remaining elements
    for (; k < numFrequencyBins_; ++k)
    {
        auto real = fftOutputBuffer_[k][0];
        auto imag = fftOutputBuffer_[k][1];
        magnitudes[k] = std::sqrt((real * real) + (imag * imag));
    }

#endif // !defined(USE_AVX2)

    return magnitudes;
}

bool AudioAnalyzer::haveStatic_(const std::vector<float>& magnitudes) const
{
    return std::all_of
    (
        magnitudes.begin(),
        magnitudes.end(),
        [](float mag)
        {
            // Static detection logic placeholder (to be implemented later)
            // For now, we assume a simple placeholder threshold
            constexpr auto static_threshold = 1000.0f;
            // ^ ALTHOUGH, maybe seems to be working well for a placeholder?
            return mag > static_threshold;
        }
    );
}
