#define USE_AVX2 // Temp

#include "AudioAnalyzer.h"
#include "Diagnostics.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <fstream>
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
        "File: {}\nChunk length (seconds): {:.2f}\nStaticky chunk start times: [{}]";

    // Format staticChunkStartTimes as a comma-separated list
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
        a.chunkDurationSeconds,
        static_chunk_start_times
    );
}

AudioAnalyzer::AudioAnalyzer()
{
    // Load the wisdom file here?

    _initFftw();
    _initWindow();
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
    std::vector<Analysis> analyses(inFiles.size());
    _process(analyses, inFiles);
    return analyses;
}

void AudioAnalyzer::_initFftw()
{
    // https://www.fftw.org/doc/SIMD-alignment-and-fftw_005fmalloc.html
    // fftwf_alloc_real & fftwf_alloc_complex are wrappers that call fftwf_malloc

    // Read/write from wisdom based on FFT size

    m_numFrequencyBins = (m_fftSize / 2) + 1;
    m_fftInputBuffer = fftwf_alloc_real(m_fftSize);
    m_fftOutputBuffer = fftwf_alloc_complex(m_numFrequencyBins);

    m_fftwPlan = fftwf_plan_dft_r2c_1d
    (
        static_cast<int>(m_fftSize),
        m_fftInputBuffer,
        m_fftOutputBuffer,
        FFTW_ESTIMATE
    );
}

void AudioAnalyzer::_freeFftw()
{
    // If we only call this in destructor, then I guess we don't need checks or
    // nullptr assignment
    if (m_fftwPlan)
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
    }
}

void AudioAnalyzer::_initWindow()
{
    // Hann window:
    for (auto i = 0; i < m_fftSize; ++i)
    {
        // Calculate the cosine of the normalized angular position for index i
        auto cosine = std::cos((2.0f * PI * i) / (m_fftSize - 1));

        // Calculate the Hann window coefficient for index i
        m_window[i] = 0.5f * (1.0f - cosine);
    }
}

// Find common elements later
// Refine into smaller functions (potentially called from both this and AVX2
// function)
void AudioAnalyzer::_process
(
    std::vector<Analysis>& analyses,
    const std::vector<std::filesystem::path>& inFiles
)
{

#if defined(USE_AVX2)

    for (std::size_t i = 0; i < inFiles.size(); ++i)
    {
        std::vector<float> static_chunk_start_times{}; // Eventual product
        auto& in_file = inFiles[i];

        // Right now, we're not stopping or handling failure in any way
        if (!std::filesystem::exists(in_file))
        {
            DX_THROW_RTE("\"{}\" does not exist.", in_file.string());
        }

        if (!std::filesystem::is_regular_file(in_file))
        {
            DX_THROW_RTE("\"{}\" is not a regular file.", in_file.string());
        }

        std::ifstream raw_audio(in_file, std::ios::binary);

        if (!raw_audio)
        {
            DX_THROW_RTE("Unable to open file at \"{}\"", in_file.string());
        }

        // Calculate raw audio stream size
        raw_audio.seekg(0, std::ios::end);
        std::streamsize raw_audio_size = raw_audio.tellg();
        raw_audio.seekg(0, std::ios::beg);

        // Calculate number of chunks
        auto total_samples = raw_audio_size / sizeof(std::int16_t);
        auto chunks_count = total_samples / m_fftSize;
        auto has_remainder = (total_samples % m_fftSize) != 0;

        // Analyze full chunks and record start time (in seconds) of chunks with static
        std::vector<std::int16_t> buffer(m_fftSize);
        for (std::size_t chunk_i = 0; chunk_i < chunks_count; ++chunk_i)
        {
            raw_audio.read(reinterpret_cast<char*>(buffer.data()), m_fftSize * sizeof(std::int16_t));
            auto chunk_start_time = static_cast<float>(chunk_i * m_fftSize) / SAMPLING_RATE;

            _fftAnalyzeChunk
            (
                buffer,
                chunk_start_time,
                static_chunk_start_times
            );
        }

        // Analyze remainder
        if (has_remainder)
        {
            std::size_t remainder_samples = total_samples % m_fftSize;
            std::vector<std::int16_t> remainder_buffer(m_fftSize, 0);
            raw_audio.read(reinterpret_cast<char*>(remainder_buffer.data()), remainder_samples * sizeof(std::int16_t));
            auto remainder_start_time = static_cast<float>(chunks_count * m_fftSize) / SAMPLING_RATE;

            _fftAnalyzeChunk
            (
                remainder_buffer,
                remainder_start_time,
                static_chunk_start_times
            );
        }

        // Aggregate results
        analyses[i].file = in_file;
        analyses[i].chunkDurationSeconds = static_cast<float>(m_fftSize) / SAMPLING_RATE;
        analyses[i].staticChunkStartTimes = static_chunk_start_times;
    }

#else // !defined(USE_AVX2)

    for (std::size_t i = 0; i < inFiles.size(); ++i)
    {
        std::vector<float> static_chunk_start_times{}; // Eventual product
        auto& in_file = inFiles[i];

        // Right now, we're not stopping or handling failure in any way
        if (!std::filesystem::exists(in_file))
        {
            DX_THROW_RTE("\"{}\" does not exist.", in_file.string());
        }

        if (!std::filesystem::is_regular_file(in_file))
        {
            DX_THROW_RTE("\"{}\" is not a regular file.", in_file.string());
        }

        std::ifstream raw_audio(in_file, std::ios::binary);

        if (!raw_audio)
        {
            DX_THROW_RTE("Unable to open file at \"{}\"", in_file.string());
        }

        // Calculate raw audio stream size
        raw_audio.seekg(0, std::ios::end);
        std::streamsize raw_audio_size = raw_audio.tellg();
        raw_audio.seekg(0, std::ios::beg);

        // Calculate number of chunks
        auto total_samples = raw_audio_size / sizeof(std::int16_t);
        auto chunks_count = total_samples / m_fftSize;
        auto has_remainder = (total_samples % m_fftSize) != 0;

        // Analyze full chunks and record start time (in seconds) of chunks with static
        std::vector<std::int16_t> buffer(m_fftSize);
        for (std::size_t chunk_i = 0; chunk_i < chunks_count; ++chunk_i)
        {
            raw_audio.read(reinterpret_cast<char*>(buffer.data()), m_fftSize * sizeof(std::int16_t));
            auto chunk_start_time = static_cast<float>(chunk_i * m_fftSize) / SAMPLING_RATE;

            _fftAnalyzeChunk
            (
                buffer,
                chunk_start_time,
                static_chunk_start_times
            );
        }

        // Analyze remainder
        if (has_remainder)
        {
            std::size_t remainder_samples = total_samples % m_fftSize;
            std::vector<std::int16_t> remainder_buffer(m_fftSize, 0);
            raw_audio.read(reinterpret_cast<char*>(remainder_buffer.data()), remainder_samples * sizeof(std::int16_t));
            auto remainder_start_time = static_cast<float>(chunks_count * m_fftSize) / SAMPLING_RATE;

            _fftAnalyzeChunk
            (
                remainder_buffer,
                remainder_start_time,
                static_chunk_start_times
            );
        }

        // Aggregate results
        analyses[i].file = in_file;
        analyses[i].chunkDurationSeconds = static_cast<float>(m_fftSize) / SAMPLING_RATE;
        analyses[i].staticChunkStartTimes = static_chunk_start_times;
    }

#endif // defined(USE_AVX2)

}

// Don't forget to section off small, reusable pieces of code to reduce the
// redundancy alllll over this right now
void AudioAnalyzer::_fftAnalyzeChunk
(
    const std::vector<std::int16_t>& chunk,
    float segmentStartTimeSeconds,
    std::vector<float>& staticChunkStartTimes
)
{

#if defined(USE_AVX2)

    // Copy chunk data into FFT input buffer with scaling and window
    for (std::size_t i = 0; i < chunk.size(); ++i)
    {
        m_fftInputBuffer[i] = chunk[i] * m_window[i];
    }

    // Zero-pad the remainder of the buffer if the chunk is smaller than m_fftSize
    std::fill
    (
        m_fftInputBuffer + chunk.size(),
        m_fftInputBuffer + m_fftSize,
        0.0f
    );

    // Execute the FFT plan
    fftwf_execute(m_fftwPlan);

    // Analyze FFT output (magnitude calculation for each frequency bin)
    std::vector<float> magnitudes(m_numFrequencyBins);
    for (std::size_t k = 0; k < m_numFrequencyBins; ++k)
    {
        auto real = m_fftOutputBuffer[k][0];
        auto imag = m_fftOutputBuffer[k][1];
        magnitudes[k] = std::sqrt((real * real) + (imag * imag));
    }

    // Static detection logic placeholder (to be implemented later)
    // For now, we assume a simple placeholder threshold
    auto static_threshold = 1000.0f; // Placeholder value

    bool is_static = std::all_of
    (
        magnitudes.begin(),
        magnitudes.end(),
        [=](float mag)
        {
            return mag > static_threshold;
        }
    );

    if (is_static)
    {
        staticChunkStartTimes.emplace_back(segmentStartTimeSeconds);
    }

#else // !defined(USE_AVX2)

    // Copy chunk data into FFT input buffer with scaling and Hann window
    for (std::size_t i = 0; i < chunk.size(); ++i)
    {
        m_fftInputBuffer[i] = chunk[i] * m_hannWindow[i];
    }

    // Zero-pad the remainder of the buffer if the chunk is smaller than m_fftSize
    std::fill
    (
        m_fftInputBuffer + chunk.size(),
        m_fftInputBuffer + m_fftSize,
        0.0f
    );

    // Execute the FFT plan
    fftwf_execute(m_fftwPlan);

    // Analyze FFT output (magnitude calculation for each frequency bin)
    std::vector<float> magnitudes(m_numFrequencyBins);
    for (std::size_t k = 0; k < m_numFrequencyBins; ++k)
    {
        auto real = m_fftOutputBuffer[k][0];
        auto imag = m_fftOutputBuffer[k][1];
        magnitudes[k] = std::sqrt((real * real) + (imag * imag));
    }

    // Static detection logic placeholder (to be implemented later)
    // For now, we assume a simple placeholder threshold
    auto static_threshold = 1000.0f; // Placeholder value

    bool is_static = std::all_of
    (
        magnitudes.begin(),
        magnitudes.end(),
        [=](float mag)
        {
            return mag > static_threshold;
        }
    );

    if (is_static)
    {
        staticChunkStartTimes.emplace_back(segmentStartTimeSeconds);
    }

#endif // defined(USE_AVX2)

}
