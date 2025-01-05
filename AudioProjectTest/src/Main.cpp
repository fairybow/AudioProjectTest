#include "AudioAnalyzer.h"
#include "VoiceDetector.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

int main(int argc, char* argv[])
{
    // argv[0] = executable path. Note, we avoid repeated allocations by
    // reserving our size and using emplace_back (vs. push_back)
    std::vector<std::filesystem::path> audio_file_paths(argc - 1);

    for (auto i = 1; i < argc; ++i)
        audio_file_paths.emplace_back(argv[i]);

    try
    {
        AudioAnalyzer analyzer{};
        auto analysis = analyzer.process("C:\\Dev\\sample-audio-file-human-then-static.raw");
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what();
        return 1;
    }
}
