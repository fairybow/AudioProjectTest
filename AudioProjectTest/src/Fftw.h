#pragma once

#include <filesystem>
#include <string>

// https://www.fftw.org/index.html

namespace Fftw
{
    // return type TBD
    std::string analyze(const std::filesystem::path& inFile);
}
