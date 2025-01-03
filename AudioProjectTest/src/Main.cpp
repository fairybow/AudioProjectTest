#include <filesystem>
#include <format>
#include <iostream>
#include <source_location>
#include <stdexcept>
#include <string>

#include "fftw3.h"

// TODO:
// Potentially build the library into the program instead
// Determine library type and implement later. For now, just make console app.
// It's also possible we only need console app.
// Separate methods out into ns/classes, obvs

// https://www.fftw.org/index.html

// https://en.wikipedia.org/wiki/Voice_frequency
// constexpr auto VOICE_HZ_MIN = 300;
// constexpr auto VOICE_HZ_MAX = 3400;

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

static void read(const std::filesystem::path& audioFile)
{
    //...
}

// Read an audio file at path `in` and return an analysis as string (or throw an
// exception)
static std::string analyze(const std::filesystem::path& in)
{
    if (!std::filesystem::exists(in))
    {
        THROW_RTE("\"{}\" does not exist.", in.string());
    }

    if (!std::filesystem::is_regular_file(in))
    {
        THROW_RTE("\"{}\" is not regular file.", in.string());
    }

    // Read audio file at path using libsndfile
    // Get component frequencies using FFTW
    // Analyze

    return {}; // temp
}

// Provide paths to sound files
int main(int argc, char* argv[])
{
    try
    {
        std::cout << analyze("C:/Dev/Nonexistent.txt");
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

#undef THROW_RTE
