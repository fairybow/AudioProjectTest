#include <filesystem>
#include <format>
#include <iostream>
#include <stdexcept>

#include "fftw3.h"

// TODO:
// Potentially build the library into the program instead
// Determine library type and implement later. For now, just make console app.
// It's also possible we only need console app.
// Separate methods out into ns/classes, obvs

// https://www.fftw.org/index.html

// https://en.wikipedia.org/wiki/Voice_frequency
//constexpr auto VOICE_HZ_MIN = 300;
//constexpr auto VOICE_HZ_MAX = 3400;

//static void die(const char* function, const char* what)
//#define DIE(...) die(__FUNCTION__, __LINE__, __VA_ARGS__) // should at least be one arg ("what" message), so no ##

static void throwRte(const char* what)
{
    throw std::runtime_error(what);
}

template <typename... ArgsT>
static void throwRteFormat(const char* format, ArgsT&&... args)
{
    auto what = std::vformat(format, std::make_format_args(args...));
    throwRte(what.c_str());
}

static void read(const std::filesystem::path& audioFile)
{
    //...
}

// Read an audio file at path `in` and return an analysis as string (or throw an exception)
static std::string analyze(const std::filesystem::path& in)
{
    if (!std::filesystem::exists(in))
    {
        throwRteFormat("\"{}\" does not exist.", in.string());
    }

    if (!std::filesystem::is_regular_file(in))
    {
        throwRteFormat("\"{}\" is not regular file.", in.string());
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
