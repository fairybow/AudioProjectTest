#pragma once

#if defined(DX_BENCH)

#pragma message("Diagnostics: Benchmarking enabled.")

#include <chrono>
#include <iostream>

#endif

#include <format>

// Separate logging/messaging namesapce
// Include it for methods to print location
// Potentially remove a lot or all (esp formatting) if it harms speed

// `std::source_location` doesn't allow us to obtain an unqualified function
// signature like __FUNCTION__. (May need to adjust for platforms.)
#define DX_THROW(error, message, ...)   \
    Diagnostics::throwError             \
    (                                   \
        error,                          \
        __FILE__,                       \
        __LINE__,                       \
        __FUNCTION__,                   \
        message,                        \
        ##__VA_ARGS__                   \
    )

#define DX_THROW_IAE(message, ...)      \
    DX_THROW                            \
    (                                   \
        Diagnostics::InvalidArg,        \
        message,                        \
        ##__VA_ARGS__                   \
    )

#define DX_THROW_OORE(message, ...)     \
    DX_THROW                            \
    (                                   \
        Diagnostics::OutOfRange,        \
        message,                        \
        ##__VA_ARGS__                   \
    )

#define DX_THROW_RTE(message, ...)      \
    DX_THROW                            \
    (                                   \
        Diagnostics::RunTime,           \
        message,                        \
        ##__VA_ARGS__                   \
    )

#if defined(DX_BENCH)

#define DX_START_BENCH_BLOCK(operationName)                                             \
    auto dx_bench_operation_name = operationName;                                       \
    auto dx_bench_start_time = std::chrono::steady_clock::now()

#define DX_END_BENCH_BLOCK                                                              \
    auto dx_bench_end_time = std::chrono::steady_clock::now();                          \
    auto dx_bench_duration = std::chrono::duration_cast<std::chrono::milliseconds>(     \
        dx_bench_end_time - dx_bench_start_time);                                       \
    std::cout << dx_bench_operation_name << " completed in "                            \
        << dx_bench_duration.count()                                                    \
        << " ms." << std::endl

#else

#define DX_START_BENCH_BLOCK(operationName)
#define DX_END_BENCH_BLOCK

#endif

namespace Diagnostics
{
    enum Error
    {
        InvalidArg,
        OutOfRange,
        RunTime
    };

    void throwError
    (
        Error error,
        const char* file,
        int line,
        const char* function,
        const char* message
    );

    template <typename... ArgsT>
    void throwError
    (
        Error error,
        const char* file,
        int line,
        const char* function,
        const char* messageFormat,
        ArgsT&&... args
    )
    {
        auto formatted_message = std::vformat(messageFormat, std::make_format_args(args...));
        throwError(error, file, line, function, formatted_message.c_str());
    }

} // namespace Diagnostics
