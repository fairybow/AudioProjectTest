#include "Windowing.h"

#include <cmath>

constexpr auto PI = 3.141593f; // Accurate enough?

namespace Windowing
{
    std::string toString(Window windowType) noexcept
    {
        switch (windowType)
        {
        case Triangular:    return "Triangular";
        case Hann:          return "Hann";
        case Hamming:       return "Hamming";
        case Blackman:      return "Blackman";
        case FlatTop:       return "FlatTop";
        case Gaussian:      return "Gaussian";

        default:
        case None:          return "None";
        }
    }

    // Untested
    std::vector<float> triangular(std::size_t size)
    {
        std::vector<float> window(size);

        for (auto i = 0; i < size; ++i)
        {
            window[i] = 1.0f - std::abs((2.0f * i) / (size - 1) - 1.0f);
        }

        return window;
    }

    std::vector<float> hann(std::size_t size)
    {
        // Hann formula
        // w[i] = 0.5 * (1 - cos( (2 * PI * i) / (N -1) ))

        // Precomputing the scale factor is algebraically equivalent to dividing
        // (2 * PI * i) by (N - 1) inside the cosine function.

        std::vector<float> window(size);
        auto scale = (2.0f * PI) / (size - 1);

        for (auto i = 0; i < size; ++i)
        {
            // Calculate the cosine of the normalized angular position for index i
            auto cosine = std::cos(scale * i);

            // Calculate the Hann window coefficient for index i
            window[i] = 0.5f * (1.0f - cosine);
        }

        return window;
    }

    // Untested
    std::vector<float> hamming(std::size_t size)
    {
        std::vector<float> window(size);
        const auto scale = 2.0f * PI / (size - 1);

        for (auto i = 0; i < size; ++i)
        {
            auto cosine = std::cos(scale * i);
            window[i] = 0.54f - 0.46f * cosine;
        }

        return window;
    }

    // Untested
    std::vector<float> blackman(std::size_t size)
    {
        std::vector<float> window(size);
        const auto scale = 2.0f * PI / (size - 1);

        for (auto i = 0; i < size; ++i)
        {
            auto cosine1 = std::cos(scale * i);
            auto cosine2 = std::cos(2.0f * scale * i);
            window[i] = 0.42f - 0.5f * cosine1 + 0.08f * cosine2;
        }

        return window;
    }

    // Untested
    std::vector<float> flatTop(std::size_t size)
    {
        std::vector<float> window(size);
        const auto scale = 2.0f * PI / (size - 1);

        for (auto i = 0; i < size; ++i)
        {
            auto cosine1 = std::cos(scale * i);
            auto cosine2 = std::cos(2.0f * scale * i);
            auto cosine3 = std::cos(3.0f * scale * i);
            window[i] = 1.0f - 1.93f * cosine1 + 1.29f * cosine2 - 0.388f * cosine3 + 0.0322f * std::cos(4.0f * scale * i);
        }

        return window;
    }

    // Untested
    std::vector<float> gaussian(std::size_t size, float sigma)
    {
        std::vector<float> window(size);
        const auto midpoint = (size - 1) / 2.0f; // Center of the window

        for (std::size_t i = 0; i < size; ++i)
        {
            // Normalize distance from the center, scaled by sigma
            auto distance = (i - midpoint) / (sigma * midpoint);

            // Gaussian window formula
            window[i] = std::exp(-0.5f * distance * distance);
        }

        return window;
    }

} // namespace Windowing
