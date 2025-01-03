#pragma once

#include <filesystem>
#include <string>

// https://www.fftw.org/index.html
// We are using FFTW's single (float) precision, for speed

namespace Fftw
{
    // return type TBD
    std::string analyze(const std::filesystem::path& inFile);
}
