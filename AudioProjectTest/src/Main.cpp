#include "AudioAnalyzer.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

// todo - parralel chunk processing
// todo - error code for analysis struct, present if processing fails, keeping
// us from throwing or whatever else
// todo - cmd line control of AudioAnalyzer object via flags (wisdom path, fft
// size, overlap, and window type args)




int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Error: No audio file paths provided." << std::endl;
        return 1;
    }

    std::vector<std::filesystem::path> audio_file_paths{};

    for (auto i = 1; i < argc; ++i)
        audio_file_paths.emplace_back(argv[i]);

    try
    {
        AudioAnalyzer analyzer("C:/Dev/fftwf_wisdom.dat"); // use flag
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
