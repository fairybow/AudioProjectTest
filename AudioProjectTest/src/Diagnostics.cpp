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
        using namespace std::chrono; // chill out, stdlib

        auto end = steady_clock::now();
        auto duration = duration_cast<milliseconds>(end - m_start);

        constexpr auto format = "{} completed in {} milliseconds.";

        auto message = std::format
        (
            format,
            m_processName,
            duration.count()
        );

        std::cout << message << std::endl;
    }

    void throwError
    (
        Error error,
        const Location& location,
        const char* message
    )
    {
        constexpr auto notation = "{}:{} ({}): {}";
        auto file_name = std::filesystem::path(location.file).filename();

        auto what = std::format
        (
            notation,
            file_name.string(),
            location.line,
            location.function,
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
