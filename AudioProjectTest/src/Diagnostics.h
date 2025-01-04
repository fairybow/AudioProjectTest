#pragma once

#include <format>

// Separate logging/messaging namesapce
// Include it for methods to print location
// Potentially remove a lot or all (esp formatting) if it harms speed

// `std::source_location` doesn't allow us to obtain an unqualified function
// signature like __FUNCTION__. (May need to adjust for platforms.)
#define DX_THROW_RTE(message, ...)      \
    Diagnostics::throwRuntimeError      \
    (                                   \
        __FILE__,                       \
        __LINE__,                       \
        __FUNCTION__,                   \
        message,                        \
        ##__VA_ARGS__                   \
    )

namespace Diagnostics
{
    void throwRuntimeError
    (
        const char* file,
        int line,
        const char* function,
        const char* message
    );

    template <typename... ArgsT>
    void throwRuntimeError
    (
        const char* file,
        int line,
        const char* function,
        const char* messageFormat,
        ArgsT&&... args
    )
    {
        auto formatted_message = std::vformat(messageFormat, std::make_format_args(args...));
        throwRuntimeError(file, line, function, formatted_message.c_str());
    }

} // namespace Diagnostics
