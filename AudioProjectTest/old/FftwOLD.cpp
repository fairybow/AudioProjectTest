/*#include "Diagnostics.h"
#include "Fftw.h"

#include "fftw3.h"

#include <fstream>
#include <ios>
#include <type_traits>
#include <vector>

//------------------------------------------------------------------------------
// Internal
//------------------------------------------------------------------------------

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

static std::streamsize _sizeOf(std::ifstream& stream)
{
    stream.seekg(0, std::ios::end);
    std::streamsize size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    return size;
}

static std::vector<float> _toPreparedSamples(const std::vector<int16_t>& rawSamples)
{
    auto fft_size = _fftSize(rawSamples);
    std::vector<float> prepared_samples(fft_size);

    // Convert and normalize raw samples
    for (size_t i = 0; i < fft_size; ++i)
        prepared_samples[i] = static_cast<float>(rawSamples[i]) / std::numeric_limits<int16_t>::max();

    // Apply a window function(e.g., Hann window) to reduce spectral leakage
    //for (size_t i = 0; i < fft_size; ++i)
    //{
    //    prepared_samples[i] *= 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (fft_size - 1)));
    //}

    return prepared_samples;
}

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

//------------------------------------------------------------------------------
// Definitions
//------------------------------------------------------------------------------

namespace Fftw
{
    std::string analyze(const std::filesystem::path& inFile)
    {
        if (!std::filesystem::exists(inFile))
            DX_THROW_RTE("\"{}\" does not exist.", inFile.string());

        if (!std::filesystem::is_regular_file(inFile))
            DX_THROW_RTE("\"{}\" is not a regular file.", inFile.string());

        std::ifstream raw_audio(inFile, std::ios::binary);
        if (!raw_audio) DX_THROW_RTE("Unable to open file at \"{}\"", inFile.string());

        auto raw_samples = _toRawSamples(raw_audio);
        auto prepared_samples = _toPreparedSamples(raw_samples);

        // Perform FFTA using FFTW

        // (Use the fftwf api, not fftw)

        // Temp ---------
        return {};
    }
}
*/
