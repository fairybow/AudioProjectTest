#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <source_location>
#include <stdexcept>
#include <string>
#include <vector>

#include "fftw3.h"

// TODO:
// Potentially build the library into the program instead.
// Determine library type and implement later. For now, just make console app.
// It's also possible we only need console app.
// Separate methods out into ns/classes, obvs.

// https://www.fftw.org/index.html

// https://en.wikipedia.org/wiki/Voice_frequency
// (Maybe:)
// constexpr auto VOICE_HZ_MIN = 300;
// constexpr auto VOICE_HZ_MAX = 3400;

// Debug -----------------------------------------------------------------------

#define THROW_RTE(message, ...)                                                \
    throwRte(std::source_location::current(), message, ##__VA_ARGS__)

static void throwRte(const std::source_location& location, const char* message)
{
    constexpr auto notation = "{}:{} ({}): {}";

    auto what = std::format(
        notation,
        location.file_name(),
        location.line(),
        location.function_name(),
        message);

    throw std::runtime_error(what.c_str());
}

template <typename... ArgsT>
static void throwRte(
    const std::source_location& location,
    const char* messageFormat,
    ArgsT&&... args)
{
    auto formatted_message =
        std::vformat(messageFormat, std::make_format_args(args...));

    throwRte(file, line, function, formatted_message.c_str());
}

// ----------------------------------------------------------------------------

static std::streamsize sizeOf(std::ifstream& stream)
{
    stream.seekg(0, std::ios::end);
    std::streamsize size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    return size;
}

static std::vector<int16_t> toSamples(std::ifstream& rawAudio)
{
    constexpr auto single_sample_size = sizeof(int16_t);
    auto file_size = sizeOf(rawAudio);

    // Ensure the file size is valid for 16-bit samples (2 bytes).
    if ((file_size % single_sample_size) != 0)
        THROW_RTE("Invalid or corrupted raw audio file.");

    // Create a vector to hold the samples, sized to the sample count
    auto sample_count = file_size / single_sample_size;
    std::vector<int16_t> samples(sample_count);

    // Read the file into the vector
    // Using reinterpret_cast allows us to read the binary data directly into a
    // `std::vector<int16_t>` without manually combining bytes. Without it, we'd
    // have to read the data as `char` or `uint8_t` and then manually combine
    // every two bytes into a single `int16_t`.
    if (!rawAudio.read(reinterpret_cast<char*>(samples.data()), file_size))
        THROW_RTE("Failed to read raw audio data.");

    return samples;
}

static std::string analyze(const std::filesystem::path& inFile)
{
    if (!std::filesystem::exists(inFile))
        THROW_RTE("\"{}\" does not exist.", inFile.string());

    if (!std::filesystem::is_regular_file(inFile))
        THROW_RTE("\"{}\" is not regular file.", inFile.string());

    std::ifstream file(inFile, std::ios::binary);

    if (!file) THROW_RTE("Unable to open file at \"{}\"", inFile.string());

    auto samples = toSamples(file);

    // Read audio file at path
    // Get component frequencies using FFTW
    // Analyze

    return {}; // temp
}

// Provide paths to sound files
int main(int argc, char* argv[])
{
    // argv[0] = executable path
    for (auto i = 1; i < argc; ++i)
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
    }
}

#undef THROW_RTE
