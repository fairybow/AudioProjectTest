#pragma once

#include <format>

// Separate logging/messaging namesapce
// Include it for methods to print location
// Potentially remove a lot or all (esp formatting) if it harms speed

// `std::source_location` doesn't allow us to obtain an unqualified function
// signature like __FUNCTION__. (May need to adjust for platforms.)
#define DX_THROW_RTE(message, ...)      \
    Diagnostics::throwError             \
    (                                   \
        Diagnostics::RunTime,           \
        __FILE__,                       \
        __LINE__,                       \
        __FUNCTION__,                   \
        message,                        \
        ##__VA_ARGS__                   \
    )

#define DX_THROW_OOR(message, ...)      \
    Diagnostics::throwError             \
    (                                   \
        Diagnostics::OutOfRange,        \
        __FILE__,                       \
        __LINE__,                       \
        __FUNCTION__,                   \
        message,                        \
        ##__VA_ARGS__                   \
    )

namespace Diagnostics
{
    enum Error
    {
        RunTime,
        OutOfRange
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
