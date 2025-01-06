#include "Diagnostics.h"

#include <filesystem>
#include <stdexcept>

namespace Diagnostics
{
    void throwError
    (
        Error error,
        const char* file,
        int line,
        const char* function,
        const char* message
    )
    {
        constexpr auto notation = "{}:{} ({}): {}";
        auto file_name = std::filesystem::path(file).filename();

        auto what = std::format
        (
            notation,
            file_name.string(),
            line,
            function,
            message
        );

        switch (error)
        {
        case OutOfRange:    throw std::out_of_range(what.c_str());

        default:
        case RunTime:       throw std::runtime_error(what.c_str());
        }
    }

} // namespace Diagnostics
