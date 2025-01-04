#include "AudioAnalyzer.h"
#include "Diagnostics.h"

#include "fftw3.h"

AudioAnalyzer::AudioAnalyzer()
{
    // Load or create the wisdom file
}

std::vector<AudioAnalyzer::Analysis> AudioAnalyzer::process(const std::vector<std::filesystem::path>& paths, size_t fftSize)
{
    //...

    return {};
};

// Convenience overload for single process
AudioAnalyzer::Analysis AudioAnalyzer::process(const std::filesystem::path& path, size_t fftSize)
{
    return process(std::vector<std::filesystem::path>{ path }, fftSize).at(0);
}
