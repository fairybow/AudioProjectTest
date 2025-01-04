#include "AudioAnalyzer.h"
#include "Diagnostics.h"

#include "fftw3.h"

AudioAnalyzer::AudioAnalyzer()
{
}

std::vector<AudioAnalyzer::Analysis> AudioAnalyzer::process(const std::vector<std::filesystem::path>& paths)
{
    //...

    return {};
};

AudioAnalyzer::Analysis AudioAnalyzer::process(const std::filesystem::path& path)
{
    return process(std::vector<std::filesystem::path>{ path }).at(0);
}
