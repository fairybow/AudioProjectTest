/*#include "AudioAnalyzer.h"
#include "Diagnostics.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>

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
    // fftSize_ Initializes to 0
    auto fft_size = static_cast<int>(fftSize);
    if (fftSize_ != fft_size)
    {
        fftSize_ = fft_size;
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
    numFrequencyBins_ = (fftSize_ / 2) + 1;
    fftInputBuffer_ = fftwf_alloc_real(fftSize_);
    fftOutputBuffer_ = fftwf_alloc_complex(numFrequencyBins_);

    fftwPlan_ = fftwf_plan_dft_r2c_1d
    (
        fftSize_,
        fftInputBuffer_,
        fftOutputBuffer_,
        FFTW_ESTIMATE
    );
}

void AudioAnalyzer::_freeFftwPlan()
{
    if (fftwPlan_)
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
    }
}

void AudioAnalyzer::_initHannWindow()
{
    m_hannWindow.resize(fftSize_);

    for (std::size_t i = 0; i < fftSize_; ++i)
    {
        // Calculate the cosine of the normalized angular position for index i
        auto cosine = std::cos((2.0f * m_pi * i) / (fftSize_ - 1));

        // Calculate the Hann window coefficient for index i
        m_hannWindow[i] = 0.5f * (1.0f - cosine);
    }
}

std::vector<float> AudioAnalyzer::_toSamples(std::ifstream& rawAudio) const
{
    // Get the total size of the raw audio file in bytes
    auto file_byte_size = sizeOf_(rawAudio);

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

std::streamsize AudioAnalyzer::sizeOf_(std::ifstream& stream) const
{
    stream.seekg(0, std::ios::end);
    std::streamsize size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    return size;
}

// first version, no cout
AudioAnalyzer::Analysis AudioAnalyzer::_analyzeFileSamples(const std::vector<float>& fileSamples) const
{
    // Divides into frames and perform FFT

    // Setup frames
    auto hop_size = static_cast<std::size_t>(fftSize_ * m_overlapPercentage);

    // Divide the audio signal into overlapping chunks of size fftSize_, with a
    // step of hop_size between each chunk, and calculate how many such chunks
    // (frames) fit into the signal.
    // Computes the number of additional frames (after the first one) that can
    // fit into the remaining signal. Adding 1 accounts for the first frame,
    // which starts at the very beginning of the signal (index 0).
    std::size_t frame_count = ((fileSamples.size() - fftSize_) / hop_size) + 1;

    // FFTW start:

    std::vector<float> total_spectral_energy(numFrequencyBins_);

    // Rows represent frames (time slices of the signal).
    // Columns represent unique frequency bins.
    // So, magnitudes[f][k] gives the magnitude of the k-th frequency bin for
    // the f-th frame
    std::vector<std::vector<float>> magnitudes(frame_count, std::vector<float>(numFrequencyBins_));

    // FFT and magnitude calculation
    for (std::size_t frame = 0; frame < frame_count; ++frame)
    {
        // Shifts the starting position by `hop_size` samples for each
        // successive frame.
        std::size_t start_index = frame * hop_size;

        // Fill input buffer with Hann window applied:
        for (std::size_t i = 0; i < fftSize_; ++i)
        {
            if ((start_index + i) < fileSamples.size())
                fftInputBuffer_[i] = fileSamples[start_index + i] * m_hannWindow[i];
            else
                fftInputBuffer_[i] = 0.0f; // Zero-pad if there are not enough samples
        }

        fftwf_execute(fftwPlan_);

        for (std::size_t bin = 0; bin < numFrequencyBins_; ++bin)
        {
            auto real = fftOutputBuffer_[bin][0];
            auto imaginary = fftOutputBuffer_[bin][1];
            auto magnitude = std::sqrt((real * real) + (imaginary * imaginary));
            magnitudes[frame][bin] = magnitude;
            total_spectral_energy[bin] += magnitude;
        }
    }

    // Aggregate results into Analysis (example: average magnitudes)
    Analysis analysis{};
    analysis.spectralEnergy = total_spectral_energy;

    // UNTESTED GPT:

    // Calculate confidence score
    float low_freq_energy = std::accumulate(total_spectral_energy.begin() + 1, total_spectral_energy.begin() + 5, 0.0f); // Approx <300 Hz
    float mid_freq_energy = std::accumulate(total_spectral_energy.begin() + 5, total_spectral_energy.begin() + 20, 0.0f); // 300-3 kHz
    float high_freq_energy = std::accumulate(total_spectral_energy.begin() + 20, total_spectral_energy.end(), 0.0f); // >3 kHz

    // Simple heuristic for human voice detection
    if (mid_freq_energy > low_freq_energy && mid_freq_energy > high_freq_energy)
    {
        analysis.containsVoice = true;
        analysis.voiceConfidence = mid_freq_energy / (low_freq_energy + high_freq_energy + mid_freq_energy);
    }
    else
    {
        analysis.containsVoice = false;
        analysis.voiceConfidence = 0.0f;
    }

    return analysis;
}

// Process into 1 second clips and determine whether each second contains voice,
// static, music, etc.
So basically the goal is to read the audio data and process it in something
like one-second intervals, for each interval using FFT to identify the
frequencies present in that audio sample, and then based on that to characterize
whether the sample contains human speech, music, or load static / screeching
noise.  As background to this project, this came about because I need to
troubleshoot a problem on one of my systems where intermittently customers have
reported hearing loud static -- it is difficult to troubleshoot based only on
customer reports so I want to instrument the system to log or alert whenever
static is present in the audio stream.
// If we want to simply log for static, does it matter what anything else is?
// That is, if there is any sort of "guaranteed" way to identify
// static/screeching/bad noise.
AudioAnalyzer::Analysis AudioAnalyzer::_analyzeFileSamples(const std::vector<float>& fileSamples) const
{
    // Divides into frames and perform FFT

    // Setup frames
    auto hop_size = static_cast<std::size_t>(fftSize_ * m_overlapPercentage);
    std::cout << "Hop size: " << hop_size << std::endl;

    // Divide the audio signal into overlapping chunks of size fftSize_
    std::size_t frame_count = ((fileSamples.size() - fftSize_) / hop_size) + 1;
    std::cout << "Frame count: " << frame_count << std::endl;

    std::vector<float> total_spectral_energy(numFrequencyBins_);

    // Initialize magnitudes matrix
    std::vector<std::vector<float>> magnitudes(frame_count, std::vector<float>(numFrequencyBins_));

    // FFT and magnitude calculation
    for (std::size_t frame = 0; frame < frame_count; ++frame)
    {
        std::size_t start_index = frame * hop_size;
        //std::cout << "Processing frame " << frame << " starting at index " << start_index << std::endl;

        // Fill input buffer with Hann window applied
        for (std::size_t i = 0; i < fftSize_; ++i)
        {
            if ((start_index + i) < fileSamples.size())
                fftInputBuffer_[i] = fileSamples[start_index + i] * m_hannWindow[i];
            else
                fftInputBuffer_[i] = 0.0f; // Zero-pad if there are not enough samples
        }

        fftwf_execute(fftwPlan_);

        for (std::size_t bin = 0; bin < numFrequencyBins_; ++bin)
        {
            auto real = fftOutputBuffer_[bin][0];
            auto imaginary = fftOutputBuffer_[bin][1];
            auto magnitude = std::sqrt((real * real) + (imaginary * imaginary));
            magnitudes[frame][bin] = magnitude;
            total_spectral_energy[bin] += magnitude;
        }

        //std::cout << "Frame " << frame << " magnitudes: ";
        //for (const auto& mag : magnitudes[frame])
        //{
            //std::cout << mag << " ";
        //}
        //std::cout << std::endl;
    }

    // Aggregate results into Analysis (example: average magnitudes)
    Analysis analysis{};
    analysis.spectralEnergy = total_spectral_energy;

    std::cout << "Total spectral energy: ";
    for (const auto& energy : total_spectral_energy)
    {
        std::cout << energy << " ";
    }
    std::cout << std::endl;

    // Calculate confidence score
    float low_freq_energy = std::accumulate(total_spectral_energy.begin() + 1, total_spectral_energy.begin() + 5, 0.0f); // Approx <300 Hz
    float mid_freq_energy = std::accumulate(total_spectral_energy.begin() + 5, total_spectral_energy.begin() + 20, 0.0f); // 300-3 kHz
    float high_freq_energy = std::accumulate(total_spectral_energy.begin() + 20, total_spectral_energy.end(), 0.0f); // >3 kHz

    std::cout << "Low frequency energy: " << low_freq_energy << std::endl;
    std::cout << "Mid frequency energy: " << mid_freq_energy << std::endl;
    std::cout << "High frequency energy: " << high_freq_energy << std::endl;

    // Log relative contributions of energy
    float total_energy = low_freq_energy + mid_freq_energy + high_freq_energy;
    std::cout << "Low freq %: " << (low_freq_energy / total_energy) * 100 << "%\n";
    std::cout << "Mid freq %: " << (mid_freq_energy / total_energy) * 100 << "%\n";
    std::cout << "High freq %: " << (high_freq_energy / total_energy) * 100 << "%\n";

    // Adjusted heuristic for human voice detection
    if (mid_freq_energy > (low_freq_energy * 0.5) && mid_freq_energy > (high_freq_energy * 0.1))
    {
        analysis.containsVoice = true;
        analysis.voiceConfidence = mid_freq_energy / total_energy;
    }
    else
    {
        analysis.containsVoice = false;
        analysis.voiceConfidence = 0.0f;
    }

    std::cout << "Contains voice: " << (analysis.containsVoice ? "true" : "false") << std::endl;
    std::cout << "Voice confidence: " << analysis.voiceConfidence << std::endl;

    return analysis;
}
*/
