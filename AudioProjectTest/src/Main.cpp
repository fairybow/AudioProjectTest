#include "Diagnostics.h"

#include "fftw3.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// TODO:
// Potentially build the library into the program instead.
// Determine library type and implement later. For now, just make console app.
// It's also possible we only need console app.
// Potentially do not throw (just notify) and allow for reading of subsequent
// audio files, if provided.
// Separate methods out into ns/classes, obvs.

// https://www.fftw.org/index.html

// https://en.wikipedia.org/wiki/Voice_frequency
// (Maybe:)
// constexpr auto VOICE_HZ_MIN = 300;
// constexpr auto VOICE_HZ_MAX = 3400;

// pvt/internal, eventually
static std::streamsize _sizeOf(std::ifstream& stream)
{
    stream.seekg(0, std::ios::end);
    std::streamsize size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    return size;
}

// pvt/internal, eventually
// Rename?
static size_t _fftSize(const std::vector<int16_t>& rawSamples)
{
    // FFT algorithms like FFTW are most efficient when fftSize is a power of 2
    // (e.g., 256, 512, 1024).

    auto raw_size = rawSamples.size();
    if (raw_size < 2) DX_THROW_RTE("Not enough samples for FFT.");

    size_t fft_size = 1;

    // Determine size as the largest power of 2 <= the input size
    while ((fft_size * 2) <= raw_size)
        fft_size *= 2;

    return fft_size;
}

// pvt/internal, eventually
static std::vector<int16_t> _toRawSamples(std::ifstream& rawAudio)
{
    constexpr auto single_sample_size = sizeof(int16_t);
    auto file_size = _sizeOf(rawAudio);

    // Ensure the file size is valid for 16-bit samples (2 bytes).
    if ((file_size % single_sample_size) != 0)
        DX_THROW_RTE("Invalid or corrupted raw audio file.");

    // Create a vector to hold the samples, sized to the sample count.
    auto sample_count = file_size / single_sample_size;
    std::vector<int16_t> raw_samples(sample_count);

    // Read the file into the vector.
    // Note to self:
    // Using reinterpret_cast allows us to read the binary data directly into a
    // `std::vector<int16_t>` without manually combining bytes. Without it, we'd
    // have to read the data as `char` or `uint8_t` and then manually combine
    // every two bytes into a single `int16_t`.
    if (!rawAudio.read(reinterpret_cast<char*>(raw_samples.data()), file_size))
        DX_THROW_RTE("Failed to read raw audio data.");

    return raw_samples;
}

// pvt/internal, eventually
static std::vector<double> _toPreparedSamples(const std::vector<int16_t>& rawSamples)
{
    auto fft_size = _fftSize(rawSamples);
    std::vector<double> prepared_samples(fft_size);

    // Convert and normalize raw samples
    for (size_t i = 0; i < fft_size; ++i)
        prepared_samples[i] = static_cast<double>(rawSamples[i]) / std::numeric_limits<int16_t>::max();

    // Apply a window function (e.g., Hann window) to reduce spectral leakage

    return prepared_samples;
}

static std::vector<double> toSamples(std::ifstream& rawAudio)
{
    auto raw_samples = _toRawSamples(rawAudio);
    return _toPreparedSamples(raw_samples);
}

static std::string analyze(const std::filesystem::path& inFile)
{
    if (!std::filesystem::exists(inFile))
        DX_THROW_RTE("\"{}\" does not exist.", inFile.string());

    if (!std::filesystem::is_regular_file(inFile))
        DX_THROW_RTE("\"{}\" is not a regular file.", inFile.string());

    std::ifstream file(inFile, std::ios::binary);
    if (!file) DX_THROW_RTE("Unable to open file at \"{}\"", inFile.string());

    auto samples = toSamples(file);
    // Prepare sample data for analysis
    // Perform FFTA using FFTW
    // Interpret the results

    return {}; // temp
}

// Provide paths to sound files
int main(int argc, char* argv[])
{
    // Test:
    try
    {
        std::cout << analyze("C:\\Dev\\sample-audio-file-human-then-static.raw");
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what();
        return 1;
    }

    // argv[0] = executable path
    /*for (auto i = 1; i < argc; ++i)
    {
        try
        {
            std::cout << analyze(argv[i]);
        }
        catch (const std::exception& ex)
        {
            std::cout << ex.what();
            return 1;
        }
    }*/
}
