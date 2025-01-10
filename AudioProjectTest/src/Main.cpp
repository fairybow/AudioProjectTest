#include "AudioAnalyzer.h"
#include "Windowing.h"

#include <cstddef>
#include <filesystem>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

// todo - more robust static detection!
// todo - an AudioAnalyzer::Config struct to pass instead of multiple args
// todo - AA should probably know/control its flags, but not parsing (and Main
// shouldn't know about Windowing)
// todo - parralel chunk processing
// todo - error code for analysis struct, present if processing fails, keeping
// us from throwing or whatever else

// Test args: "--wisdom=C:/Dev/fftwf_wisdom.dat" "C:/Dev/sample-audio-file-human-then-static.raw"

static void parseArgs
(
    int argc,
    char* argv[],
    std::map<std::string, std::string>& flags,
    std::vector<std::filesystem::path>& paths
);

static std::size_t fftSizeFlagValue(const std::map<std::string, std::string>& flags);
static Windowing::Window windowTypeFlagValue(const std::map<std::string, std::string>& flags);
static float overlapFlagValue(const std::map<std::string, std::string>& flags);
static std::filesystem::path wisdomFlagValue(const std::map<std::string, std::string>& flags);

int main(int argc, char* argv[])
{
    std::map<std::string, std::string> flags{};
    std::vector<std::filesystem::path> audio_file_paths{};

    parseArgs
    (
        argc,
        argv,
        flags,
        audio_file_paths
    );

    try
    {
        AudioAnalyzer analyzer
        (
            fftSizeFlagValue(flags),
            windowTypeFlagValue(flags),
            overlapFlagValue(flags),
            wisdomFlagValue(flags)
        );

        auto analyses = analyzer.process(audio_file_paths);

        for (auto& analysis : analyses)
            std::cout << analysis << std::endl;
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what();
        return 1;
    }
}

void parseArgs
(
    int argc,
    char* argv[],
    std::map<std::string, std::string>& flags,
    std::vector<std::filesystem::path>& paths
)
{
    for (auto i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        // Flags
        if (arg.rfind("--", 0) == 0)
        {
            // Key-Value flags, then boolean flags
            auto equal_pos = arg.find('=');

            if (equal_pos != std::string::npos)
            {
                auto key = arg.substr(2, equal_pos - 2);
                auto value = arg.substr(equal_pos + 1);
                flags[key] = value;
            }
            else
                flags[arg.substr(2)] = "true";
        }
        else // (arg.rfind("--", 0) != 0)
        {
            // Anything else should be a path
            paths.emplace_back(arg);
        }
    }
}

std::size_t fftSizeFlagValue(const std::map<std::string, std::string>& flags)
{
    auto it = flags.find("fft-size");

    if (it != flags.end())
        return std::stoull(it->second);

    return AudioAnalyzer::DEFAULT_FFT_SIZE;
}

Windowing::Window windowTypeFlagValue(const std::map<std::string, std::string>& flags)
{
    auto it = flags.find("window");

    if (it != flags.end())
        return Windowing::fromString(it->second);

    return AudioAnalyzer::DEFAULT_WINDOW;
}

float overlapFlagValue(const std::map<std::string, std::string>& flags)
{
    auto it = flags.find("overlap");

    if (it != flags.end())
        return std::stof(it->second);

    return AudioAnalyzer::DEFAULT_OVERLAP;
}

std::filesystem::path wisdomFlagValue(const std::map<std::string, std::string>& flags)
{
    auto it = flags.find("wisdom");

    if (it != flags.end())
        return it->second;

    return {};
}
