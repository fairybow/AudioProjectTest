#include "Diagnostics.h"

#include <filesystem>
#include <stdexcept>

namespace Diagnostics
{
    void throwRuntimeError
    (
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

        throw std::runtime_error(what.c_str());
    }

} // namespace Diagnostics
