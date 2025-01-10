#include "Diagnostics.h"
#include "Windowing.h"

#include <algorithm>
#include <cmath>
#include <numbers>

// Could use function, but would miss specific notated location from DX macro.
// Although, user would know what window they were using, so maybe that's fine.
// Also, is 2 the proper lower limit for a window? I'm sure it's way more
// complicated than that...
#define THROW_IF_BAD_SIZE \
    if (size < 2) DX_THROW_INVALID_ARG("Insufficient window size")

constexpr auto NONE = "None";
constexpr auto TRIANGULAR = "Triangular";
constexpr auto HANN = "Hann";
constexpr auto HAMMING = "Hamming";
constexpr auto BLACKMAN = "Blackman";
constexpr auto FLAT_TOP = "FlatTop";
constexpr auto GAUSSIAN = "Gaussian";
constexpr auto PI = std::numbers::pi_v<float>;

static std::string normalize(const std::string& string)
{
    std::string normalized = string;
    auto first_c = true;

    std::transform
    (
        normalized.begin(),
        normalized.end(),
        normalized.begin(),
        [&first_c](unsigned char c) -> unsigned char
        {
            auto is_alpha = std::isalpha(c);

            if (first_c && is_alpha)
            {
                first_c = false;
                return std::toupper(c);
            }

            // Though, there should be no spaces for the result to matter.
            return is_alpha ? std::tolower(c) : c;
        }
    );

    return normalized;
}

namespace Windowing
{
    std::string toString(Window windowType) noexcept
    {
        switch (windowType)
        {
        case Triangular:    return TRIANGULAR;
        case Hann:          return HANN;
        case Hamming:       return HAMMING;
        case Blackman:      return BLACKMAN;
        case FlatTop:       return FLAT_TOP;
        case Gaussian:      return GAUSSIAN;

        default:
        case None:          return NONE;
        }
    }

    Window fromString(const std::string& string) noexcept
    {
        auto normalized = normalize(string);

        if (normalized == TRIANGULAR)       return Triangular;
        else if (normalized == HANN)        return Hann;
        else if (normalized == HAMMING)     return Hamming;
        else if (normalized == BLACKMAN)    return Blackman;
        else if (normalized == FLAT_TOP)    return FlatTop;
        else if (normalized == GAUSSIAN)    return Gaussian;
        else                                return None;
    }

    // Untested
    std::vector<float> triangular(std::size_t size)
    {
        THROW_IF_BAD_SIZE;

        std::vector<float> window(size);

        for (auto i = 0; i < size; ++i)
        {
            window[i] = 1.0f - std::abs((2.0f * i) / (size - 1) - 1.0f);
        }

        return window;
    }

    std::vector<float> hann(std::size_t size)
    {
        THROW_IF_BAD_SIZE;

        // Hann formula
        // w[i] = 0.5 * (1 - cos( (2 * PI * i) / (N -1) ))

        // Precomputing the scale factor is algebraically equivalent to dividing
        // (2 * PI * i) by (N - 1) inside the cosine function.

        std::vector<float> window(size);
        const auto scale = (2.0f * PI) / (size - 1);

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
        THROW_IF_BAD_SIZE;

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
        THROW_IF_BAD_SIZE;

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
        THROW_IF_BAD_SIZE;

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
        THROW_IF_BAD_SIZE;

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

#undef THROW_IF_BAD_SIZE
