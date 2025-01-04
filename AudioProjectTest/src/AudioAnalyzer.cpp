#include "AudioAnalyzer.h"
#include "Diagnostics.h"

#include "fftw3.h"

AudioAnalyzer::AudioAnalyzer()
{
    // Load the wisdom file (if present)
    // Adjust it throughout the process, if needed, for different configurations.
    // Right now, this may only be necessary for different FFT sizes?
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
