#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace Windowing
{
    enum Window
    {
        None = 0,
        Triangular,
        Hann,
        Hamming,
        Blackman,
        FlatTop,
        Gaussian
    };

    std::string toString(Window windowType) noexcept;
    std::vector<float> triangular(std::size_t size);
    std::vector<float> hann(std::size_t size);
    std::vector<float> hamming(std::size_t size);
    std::vector<float> blackman(std::size_t size);
    std::vector<float> flatTop(std::size_t size);
    std::vector<float> gaussian(std::size_t size, float sigma = 0.4f);

} // namespace Windowing
