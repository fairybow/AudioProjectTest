#include "AudioAnalyzer.h"
#include "Diagnostics.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>

AudioAnalyzer::AudioAnalyzer()
{
    // Load the wisdom file (if present)
    // Adjust it throughout the process, if needed, for different configurations.
    // Right now, this may only be necessary for different FFT sizes?
}

AudioAnalyzer::~AudioAnalyzer()
{
    _freeFftwPlan();
}

std::vector<AudioAnalyzer::Analysis> AudioAnalyzer::process(const std::vector<std::filesystem::path>& inFiles, std::size_t fftSize)
{
    // m_fftSize Initializes to 0
    if (m_fftSize != fftSize)
    {
        m_fftSize = fftSize;
        _initFftwPlan();
        _initHannWindow();
    }

    auto files_size = inFiles.size();
    std::vector<Analysis> analyses(files_size);

    for (auto i = 0; i < files_size; ++i)
    {
        auto& in_file = inFiles[i];

        // Right now, we're not stopping or handling failure in any way
        if (!std::filesystem::exists(in_file))
            std::cout << "file does not exist";

        if (!std::filesystem::is_regular_file(in_file))
            std::cout << "file is not a regular file";

        std::ifstream raw_audio(in_file, std::ios::binary);
        if (!raw_audio) std::cout << "unable to open file";

        auto file_samples = _toSamples(raw_audio);
        analyses[i] = _analyzeFileSamples(file_samples);
    }

    // Anything else?

    return analyses;
};

// Convenience overload for single process
AudioAnalyzer::Analysis AudioAnalyzer::process(const std::filesystem::path& inFile, std::size_t fftSize)
{
    return process(std::vector<std::filesystem::path>{ inFile }, fftSize).at(0);
}

void AudioAnalyzer::_initFftwPlan()
{
    // Should we avoid freeing on the first call to this on startup?
    _freeFftwPlan();

    // Read/write from wisdom:
    m_numFrequencyBins = (m_fftSize / 2) + 1;
    m_fftInputBuffer = fftwf_alloc_real(m_fftSize);
    m_fftOutputBuffer = fftwf_alloc_complex(m_numFrequencyBins);

    m_fftwPlan = fftwf_plan_dft_r2c_1d
    (
        m_fftSize,
        m_fftInputBuffer,
        m_fftOutputBuffer,
        FFTW_ESTIMATE
    );
}

void AudioAnalyzer::_freeFftwPlan()
{
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

void AudioAnalyzer::_initHannWindow()
{
    m_hannWindow.resize(m_fftSize);

    for (std::size_t i = 0; i < m_fftSize; ++i)
    {
        // Calculate the cosine of the normalized angular position for index i
        auto cosine = std::cos((2.0f * m_pi * i) / (m_fftSize - 1));

        // Calculate the Hann window coefficient for index i
        m_hannWindow[i] = 0.5f * (1.0f - cosine);
    }
}

std::vector<float> AudioAnalyzer::_toSamples(std::ifstream& rawAudio) const
{
    // Get the total size of the raw audio file in bytes
    auto file_byte_size = _sizeOf(rawAudio);

    // Size of a single audio sample in bytes (16-bit linear PCM)
    constexpr auto sample_byte_size = sizeof(std::int16_t);

    // Ensure the file size is valid for 16-bit samples
    if ((file_byte_size % sample_byte_size) != 0)
        std::cout << "Invalid or corrupted raw audio file.";

    // Calculate the number of samples in the file
    auto file_sample_count = file_byte_size / sample_byte_size;

    // Vector to store raw audio samples as 16-bit integers
    std::vector<std::int16_t> raw_audio_samples(file_sample_count);

    // Read the raw data into the vector
    if (!rawAudio.read(reinterpret_cast<char*>(raw_audio_samples.data()), file_byte_size))
        std::cout << "Failed to read raw audio data.";

    // Convert raw samples to normalized float samples
    std::vector<float> samples(file_sample_count);

    // Scale factor for normalizing 16-bit audio samples to [-1.0, 1.0]
    constexpr auto normalize = [](std::int16_t sample)
        {
            return sample * m_normalizationFactor;
        };

    std::transform
    (
        raw_audio_samples.begin(),
        raw_audio_samples.end(),
        samples.begin(),
        normalize
    );

    return samples;
}

std::streamsize AudioAnalyzer::_sizeOf(std::ifstream& stream) const
{
    stream.seekg(0, std::ios::end);
    std::streamsize size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    return size;
}

AudioAnalyzer::Analysis AudioAnalyzer::_analyzeFileSamples(const std::vector<float>& fileSamples) const
{
    // Divides into frames and perform FFT

    // Setup frames
    auto hop_size = static_cast<std::size_t>(m_fftSize * m_overlapPercentage);

    // Divide the audio signal into overlapping chunks of size m_fftSize, with a
    // step of hop_size between each chunk, and calculate how many such chunks
    // (frames) fit into the signal.
    // Computes the number of additional frames (after the first one) that can
    // fit into the remaining signal. Adding 1 accounts for the first frame,
    // which starts at the very beginning of the signal (index 0).
    std::size_t frame_count = ((fileSamples.size() - m_fftSize) / hop_size) + 1;

    // FFTW start:

    // Rows represent frames (time slices of the signal).
    // Columns represent unique frequency bins.
    // So, magnitudes[f][k] gives the magnitude of the k-th frequency bin for
    // the f-th frame
    std::vector<std::vector<float>> magnitudes(frame_count, std::vector<float>(m_numFrequencyBins));

    for (std::size_t frame = 0; frame < frame_count; ++frame)
    {
        // Shifts the starting position by `hop_size` samples for each
        // successive frame.
        std::size_t start_index = frame * hop_size;

        // Apply window
        for (std::size_t i = 0; i < m_fftSize; ++i)
            m_fftInputBuffer[i] = fileSamples[start_index + i] * m_hannWindow[i];

        fftwf_execute(m_fftwPlan);

        // Compute the inner magnitude vector (magnitude spectrum)
        for (std::size_t bin = 0; bin < m_numFrequencyBins; ++bin)
        {
            auto real_part = m_fftOutputBuffer[bin][0];
            auto imaginary_part = m_fftOutputBuffer[bin][1];
            magnitudes[frame][bin] = std::sqrt((real_part * real_part) + (imaginary_part * imaginary_part));
        }
    }

    // Compute average magnitudes
    /*std::vector<float> average_magnitudes(m_numFrequencyBins);
    for (std::size_t bin = 0; bin < m_numFrequencyBins; ++bin)
    {
        auto sum = 0.0f;

        for (std::size_t frame = 0; frame < frame_count; ++frame)
            sum += magnitudes[frame][bin];

        average_magnitudes[bin] = sum / static_cast<float>(frame_count);
    }*/

    // Aggregate results into Analysis (example: average magnitudes)
    Analysis analysis{};
    analysis.fftSize = m_fftSize;

    //...

    return analysis;
}
