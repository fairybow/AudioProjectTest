#pragma once

#include <chrono>
#include <format>
#include <memory>

// Separate logging/messaging namesapce
// Include it for methods to print location
// Potentially remove a lot or all (esp formatting) if it harms speed

// `std::source_location` doesn't allow us to obtain an unqualified function
// signature like __FUNCTION__. (May need to adjust for platforms.)
#define DX_THROW(error, message, ...)           \
    Diagnostics::throwError                     \
    (                                           \
        error,                                  \
        __FILE__,                               \
        __LINE__,                               \
        __FUNCTION__,                           \
        message,                                \
        ##__VA_ARGS__                           \
    )

#define DX_THROW_INVALID_ARG(message, ...)      \
    DX_THROW                                    \
    (                                           \
        Diagnostics::InvalidArg,                \
        message,                                \
        ##__VA_ARGS__                           \
    )

#define DX_THROW_OUT_OF_RANGE(message, ...)     \
    DX_THROW                                    \
    (                                           \
        Diagnostics::OutOfRange,                \
        message,                                \
        ##__VA_ARGS__                           \
    )

#define DX_THROW_RUN_TIME(message, ...)         \
    DX_THROW                                    \
    (                                           \
        Diagnostics::RunTime,                   \
        message,                                \
        ##__VA_ARGS__                           \
    )

#if defined(USE_DX_BENCH_MACROS)

#define DX_BENCH(processName)                   \
    std::unique_ptr<Diagnostics::Bench> DX_BENCH_##processName = std::make_unique<Diagnostics::Bench>(#processName)

// Scope will naturally kill the unique_ptr (and its underlying pointer), but
// just in case we want to kill the underlying pointer early, for some reason
#define DX_BENCH_STOP(processName)              \
    DX_BENCH_##processName.reset()

#else // !defined(USE_DX_BENCH_MACROS)

#define DX_BENCH(processName)
#define DX_BENCH_STOP(processName)

#endif // defined(USE_DX_BENCH_MACROS)

namespace Diagnostics
{
    class Bench
    {
    public:
        explicit Bench(const char* processName = "Process");
        virtual ~Bench();

    private:
        const char* m_processName;
        std::chrono::steady_clock::time_point m_start;
    };

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
