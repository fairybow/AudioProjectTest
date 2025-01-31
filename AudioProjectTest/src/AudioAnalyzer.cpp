#include "AudioAnalyzer.h"
#include "Logging.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <ios>
#include <string>

#if defined(USE_AVX2)

// AVX2 can accelerate operations involving repetitive mathematical computations
// on vectors.
#pragma message("Using AVX2 SIMD.")

#include <immintrin.h>

#endif

std::ostream& operator<<(std::ostream& os, const AudioAnalyzer::Analysis& a)
{
    constexpr auto format = \
        "File: {}\n"                            \
        "FFT Size: {}\n"                        \
        "Windowing: {}\n"                       \
        "Overlap: {}\n"                         \
        "Chunk length (seconds): {:.2f}\n"      \
        "Staticky chunk start times: [{}]";

    // Format staticChunkStartTimes as a comma-separated list
    // Silo, change to avoid repeat allocations? Worth it?
    std::string static_chunk_start_times{};
    for (std::size_t i = 0; i < a.staticChunkStartTimes.size(); ++i)
    {
        if (i > 0) static_chunk_start_times += ", ";
        static_chunk_start_times += std::format("{:.2f}", a.staticChunkStartTimes[i]);
    }

    return os << std::format
    (
        format,
        a.file.string(),
        a.fftSize,
        Windowing::toString(a.windowType),
        std::format("{}%", a.overlapDecPercent * 100.0f),
        a.chunkDurationSeconds,
        static_chunk_start_times
    );
}

AudioAnalyzer::AudioAnalyzer
(
    std::size_t fftSize,
    Windowing::Window windowType,
    float overlap,
    const std::filesystem::path& wisdomPath
)
    : m_fftSize(std::max(std::size_t(1), fftSize))
    , m_windowType(windowType)
    , m_overlapDecPercent(std::clamp(overlap, 0.0f, 0.9f))
    , m_wisdomPath(wisdomPath)
{
    _initFftw();
    _initWindow();
}

AudioAnalyzer::AudioAnalyzer(const std::filesystem::path& wisdomPath)
    : AudioAnalyzer(DEFAULT_FFT_SIZE, DEFAULT_WINDOW, DEFAULT_OVERLAP, wisdomPath)
{
}

AudioAnalyzer::~AudioAnalyzer()
{
    _freeFftw();
}

// Convenience overload for single process
AudioAnalyzer::Analysis AudioAnalyzer::process(const std::filesystem::path& inFile)
{
    return process(std::vector<std::filesystem::path>{ inFile }).at(0);
}

std::vector<AudioAnalyzer::Analysis> AudioAnalyzer::process(const std::vector<std::filesystem::path>& inFiles)
{
    // Note, this will still print if an error throws. Is that a problem?
    DX_BENCH(Processing);

    std::vector<Analysis> analyses(inFiles.size());
    _process(analyses, inFiles);
    return analyses;
}

// Section off wisdom read/write
void AudioAnalyzer::_initFftw()
{
    // https://www.fftw.org/doc/SIMD-alignment-and-fftw_005fmalloc.html
    // fftwf_alloc_real & fftwf_alloc_complex are wrappers that call
    // fftwf_malloc

    m_numFrequencyBins = (m_fftSize / 2) + 1;
    m_fftInputBuffer = fftwf_alloc_real(m_fftSize);
    m_fftOutputBuffer = fftwf_alloc_complex(m_numFrequencyBins);

    if (!m_fftInputBuffer || !m_fftOutputBuffer)
    {
        DX_THROW_RUN_TIME("Failed to allocate FFT buffers.");
    }

    auto fft_size = static_cast<int>(m_fftSize);

    if (!m_wisdomPath.empty())
    {
        // https://fftw.org/fftw3_doc/Words-of-Wisdom_002dSaving-Plans.html

        // Try to load wisdom from file
        auto wisdom_path_str = m_wisdomPath.string();
        auto wisdom_found = static_cast<bool>(
            fftwf_import_wisdom_from_filename(wisdom_path_str.c_str()));

        wisdom_found
            ? LOGGING_COUT("Wisdom file loaded from \"{}\"", wisdom_path_str)
            : LOGGING_CERR("Failed to find wisdom file at \"{}\"", wisdom_path_str);

        m_fftwPlan = fftwf_plan_dft_r2c_1d
        (
            fft_size,
            m_fftInputBuffer,
            m_fftOutputBuffer,
            FFTW_MEASURE
        );

        if (!std::filesystem::exists(m_wisdomPath.parent_path()))
        {
            std::filesystem::create_directories(m_wisdomPath.parent_path());
        }

        // Double check that this saves appropriately with variable FFT size,
        // overlap, etc. It should save only when an optimzed plan for the
        // current values wasn't found.
        if (!wisdom_found)
        {
            fftwf_export_wisdom_to_filename(wisdom_path_str.c_str())
                ? LOGGING_COUT("Wisdom file saved to \"{}\"", wisdom_path_str)
                : LOGGING_CERR("Failed to save wisdom to disk (\"{}\")", wisdom_path_str);
        }
    }
    else // (m_wisdomPath.empty())
    {
        m_fftwPlan = fftwf_plan_dft_r2c_1d
        (
            fft_size,
            m_fftInputBuffer,
            m_fftOutputBuffer,
            FFTW_ESTIMATE
        );
    }
}

void AudioAnalyzer::_freeFftw()
{
    // If we only call this in destructor, then I guess we don't need checks or
    // nullptr assignment
    /*if (m_fftwPlan)
    {
        fftwf_destroy_plan(m_fftwPlan);
        m_fftwPlan = nullptr;
    }

    if (m_fftOutputBuffer)
    {
        fftwf_free(m_fftOutputBuffer);
        m_fftOutputBuffer = nullptr;
    }

    if (m_fftInputBuffer)
    {
        fftwf_free(m_fftInputBuffer);
        m_fftInputBuffer = nullptr;
    }*/

    fftwf_destroy_plan(m_fftwPlan);
    fftwf_free(m_fftOutputBuffer);
    fftwf_free(m_fftInputBuffer);
}

void AudioAnalyzer::_initWindow()
{
    // Can perhaps get some benefit from testing different window types.
    // Ultimately, may only need one.
    switch (m_windowType)
    {
    case Windowing::Triangular:
        m_window = Windowing::triangular(m_fftSize);
        break;
    case Windowing::Hann:
        m_window = Windowing::hann(m_fftSize);
        break;
    case Windowing::Hamming:
        m_window = Windowing::hamming(m_fftSize);
        break;
    case Windowing::Blackman:
        m_window = Windowing::blackman(m_fftSize);
        break;
    case Windowing::FlatTop:
        m_window = Windowing::flatTop(m_fftSize);
        break;
    case Windowing::Gaussian:
        m_window = Windowing::gaussian(m_fftSize);
        break;

    default:
    case Windowing::None:
        m_useWindowing = false;
        break;
    }
}

// Parallelize FFT Computation?
// Find common elements later
// Refine into smaller functions (potentially called from both this and AVX2
// function)
void AudioAnalyzer::_process
(
    std::vector<Analysis>& analyses,
    const std::vector<std::filesystem::path>& inFiles
)
{
    if (inFiles.empty())
    {
        DX_THROW_INVALID_ARG("No input files provided.");
    }

    for (std::size_t i = 0; i < inFiles.size(); ++i)
    {
        std::vector<float> static_chunk_start_times{}; // Eventual product
        auto& in_file = inFiles[i];

        // Should we throw or just continue (and add an error enum to result
        // Analysis for this file, or something)
        if (!std::filesystem::exists(in_file))
        {
            DX_THROW_RUN_TIME("\"{}\" does not exist.", in_file.string());
        }

        if (!std::filesystem::is_regular_file(in_file))
        {
            DX_THROW_RUN_TIME("\"{}\" is not a regular file.", in_file.string());
        }

        std::ifstream raw_audio(in_file, std::ios::binary);

        if (!raw_audio)
        {
            DX_THROW_RUN_TIME("Unable to open file at \"{}\"", in_file.string());
        }

        // Calculate raw audio stream size
        auto raw_audio_size = _sizeOf(raw_audio);

        // Calculate number of chunks
        // Before separating this off, we will need other variables in it
        // (chunks_count, for example)...
        auto hop_size = static_cast<std::size_t>(m_fftSize * (1.0f - m_overlapDecPercent));
        auto total_samples = raw_audio_size / sizeof(std::int16_t);

        // Handle edge case where total_samples < m_fftSize
        auto chunks_count = (total_samples > m_fftSize)
            ? (((total_samples - m_fftSize) / hop_size) + 1)
            : 1;

        if (chunks_count < 1)
        {
            DX_THROW_RUN_TIME("Lol what");
        }

        std::vector<std::int16_t> buffer(m_fftSize);

        if (chunks_count == 1)
        {
            raw_audio.read
            (
                reinterpret_cast<char*>(buffer.data()),
                total_samples * sizeof(std::int16_t)
            );

            _fftAnalyzeChunk
            (
                buffer,
                0.0f,
                static_chunk_start_times,
                IsLastChunk::Yes
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
                    m_fftSize * sizeof(std::int16_t)
                );

                auto chunk_start_time = static_cast<float>(chunk_i * hop_size) / SAMPLING_RATE;

                _fftAnalyzeChunk
                (
                    buffer,
                    chunk_start_time,
                    static_chunk_start_times
                );

                // Seek back to account for overlap
                // Sliding buffer instead?
                raw_audio.seekg
                (
                    -static_cast<std::streamoff>(m_fftSize - hop_size) * sizeof(std::int16_t),
                    std::ios::cur
                );
            }

            // Analyze remainder
            if (has_remainder)
            {
                std::size_t remainder_samples = total_samples - (chunks_count * hop_size);
                std::vector<std::int16_t> remainder_buffer(m_fftSize, 0);

                raw_audio.read
                (
                    reinterpret_cast<char*>(remainder_buffer.data()),
                    remainder_samples * sizeof(std::int16_t)
                );

                auto remainder_start_time = static_cast<float>(chunks_count * hop_size) / SAMPLING_RATE;

                _fftAnalyzeChunk
                (
                    remainder_buffer,
                    remainder_start_time,
                    static_chunk_start_times,
                    IsLastChunk::Yes
                );
            }
        }

        // Aggregate results
        analyses[i] =
        {
            in_file,
            m_fftSize,
            m_windowType,
            m_overlapDecPercent,
            static_cast<float>(m_fftSize) / SAMPLING_RATE,
            static_chunk_start_times
        };
    }
}

void AudioAnalyzer::_fftAnalyzeChunk
(
    const std::vector<std::int16_t>& chunk,
    float segmentStartTimeSeconds,
    std::vector<float>& staticChunkStartTimes,
    IsLastChunk isLastChunk
)
{
    _prepareInputBuffer(chunk);

    if (isLastChunk == IsLastChunk::Yes)
    {
        _zeroPadInputBuffer(chunk);
    }

    fftwf_execute(m_fftwPlan);

    auto magnitudes = _magnitudesFromOutputBuffer();

    if (_haveStatic(magnitudes))
    {
        staticChunkStartTimes.emplace_back(segmentStartTimeSeconds);
    }
}

std::streamsize AudioAnalyzer::_sizeOf(std::ifstream& rawAudio) const
{
    rawAudio.seekg(0, std::ios::end);
    std::streamsize raw_audio_size = rawAudio.tellg();
    rawAudio.seekg(0, std::ios::beg);
    return raw_audio_size;
}

// Copy chunk data into FFT input buffer with scaling and Hann window
// Add optional windows and an option for none
void AudioAnalyzer::_prepareInputBuffer(const std::vector<std::int16_t>& chunk)
{

#if !defined(USE_AVX2)

    // Checking outside the for loop faster? Negligible, I assume?
    if (m_useWindowing)
    {
        for (std::size_t i = 0; i < chunk.size(); ++i)
            m_fftInputBuffer[i] = chunk[i] * m_window[i];
    }
    else
    {
        for (std::size_t i = 0; i < chunk.size(); ++i)
            m_fftInputBuffer[i] = chunk[i];
    }

#else // defined(USE_AVX2)

    const auto chunk_size = chunk.size();
    std::size_t i = 0;

    if (m_useWindowing)
    {
        // Process 8 elements at a time
        for (; i + 7 < chunk_size; i += 8)
        {
            // Load 8 int16_t values and extend to int32_t
            auto chunk_vals_16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&chunk[i]));
            auto chunk_vals = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(chunk_vals_16));

            // Load Hann window coefficients
            auto window_vals = _mm256_loadu_ps(&m_window[i]);

            // Perform element-wise multiplication
            auto result = _mm256_mul_ps(chunk_vals, window_vals);

            // Store the results
            _mm256_storeu_ps(&m_fftInputBuffer[i], result);
        }
    }
    else // (!m_useWindowing)
    {
        for (; i + 7 < chunk_size; i += 8)
        {
            auto chunk_vals_16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&chunk[i]));
            auto chunk_vals = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(chunk_vals_16));

            // Store the results directly
            _mm256_storeu_ps(&m_fftInputBuffer[i], chunk_vals);
        }
    }

    // Process remaining elements
    if (m_useWindowing)
    {
        for (; i < chunk_size; ++i)
            m_fftInputBuffer[i] = chunk[i] * m_window[i];
    }
    else
    {
        for (; i < chunk_size; ++i)
            m_fftInputBuffer[i] = chunk[i];
    }

#endif // !defined(USE_AVX2)

}

// Zero-pad the remainder of the buffer if the chunk is smaller than
// m_fftSize (which I would assume is almost always the case)
void AudioAnalyzer::_zeroPadInputBuffer(const std::vector<std::int16_t>& chunk)
{

#if !defined(USE_AVX2)

    std::fill
    (
        m_fftInputBuffer + chunk.size(),
        m_fftInputBuffer + m_fftSize,
        0.0f
    );

#else // defined(USE_AVX2)

    auto zero_vec = _mm256_setzero_ps();
    std::size_t j = chunk.size();
    for (; j + 7 < m_fftSize; j += 8)
    {
        _mm256_storeu_ps(&m_fftInputBuffer[j], zero_vec);
    }

    // Process remaining elements
    for (; j < m_fftSize; ++j)
    {
        m_fftInputBuffer[j] = 0.0f;
    }

#endif // !defined(USE_AVX2)

}

// Analyze FFT output (magnitude calculation for each frequency bin)
std::vector<float> AudioAnalyzer::_magnitudesFromOutputBuffer() const
{
    std::vector<float> magnitudes(m_numFrequencyBins);

#if !defined(USE_AVX2)

    for (std::size_t k = 0; k < m_numFrequencyBins; ++k)
    {
        auto real = m_fftOutputBuffer[k][0];
        auto imag = m_fftOutputBuffer[k][1];
        magnitudes[k] = std::sqrt((real * real) + (imag * imag));
    }

#else // defined(USE_AVX2)

    std::size_t k = 0;
    for (; k + 7 < m_numFrequencyBins; k += 8)
    {
        auto real_vals = _mm256_loadu_ps(&m_fftOutputBuffer[k][0]);
        auto imag_vals = _mm256_loadu_ps(&m_fftOutputBuffer[k][1]);

        auto real_sq = _mm256_mul_ps(real_vals, real_vals);
        auto imag_sq = _mm256_mul_ps(imag_vals, imag_vals);

        auto magnitude = _mm256_sqrt_ps(_mm256_add_ps(real_sq, imag_sq));
        _mm256_storeu_ps(&magnitudes[k], magnitude);
    }

    // Process remaining elements
    for (; k < m_numFrequencyBins; ++k)
    {
        auto real = m_fftOutputBuffer[k][0];
        auto imag = m_fftOutputBuffer[k][1];
        magnitudes[k] = std::sqrt((real * real) + (imag * imag));
    }

#endif // !defined(USE_AVX2)

    return magnitudes;
}

bool AudioAnalyzer::_haveStatic(const std::vector<float>& magnitudes) const
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
