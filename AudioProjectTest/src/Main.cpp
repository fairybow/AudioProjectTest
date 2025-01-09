#include "AudioAnalyzer.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

// todo - parralel chunk processing
// todo - cmd line control of AudioAnalyzer object via flags (wisdom path, fft
// size, overlap, and window type args)

int main(int argc, char* argv[])
{
    std::vector<std::filesystem::path> audio_file_paths(argc - 1);

    for (auto i = 1; i < argc; ++i)
        audio_file_paths.emplace_back(argv[i]);

    try
    {
        AudioAnalyzer analyzer("C:/Dev/fftwf_wisdom.dat");
        auto analysis = analyzer.process("C:/Dev/sample-audio-file-human-then-static.raw");
        std::cout << analysis << std::endl;

        /*auto analyses = analyzer.process(audio_file_paths);

        for (auto& analysis : analyses)
            std::cout << analysis << std::endl;*/
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what();
        return 1;
    }
}
