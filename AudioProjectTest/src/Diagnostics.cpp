#include "Diagnostics.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace Diagnostics
{
    Bench::Bench(const char* processName)
        : m_processName(processName)
        , m_start(std::chrono::steady_clock::now())
    {
    }

    Bench::~Bench()
    {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start);
        std::cout << m_processName << " completed in " << duration.count() << " ms." << std::endl;
    }

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
