#pragma once

#include <format>

#if defined(USE_LOGGING)

#define LOGGING_CERR(message, ...)             \
    Logging::cerr(message, ##__VA_ARGS__)

#define LOGGING_COUT(message, ...)             \
    Logging::cout(message, ##__VA_ARGS__)

#else // !defined(USE_LOGGING)

#define LOGGING_CERR(message, ...) ((void)0)
#define LOGGING_COUT(message, ...) ((void)0)

#endif // defined(USE_LOGGING)

namespace Logging
{
    void cerr(const char* message);
    void cout(const char* message);

    // Combine these
    template <typename... ArgsT>
    void cerr(const char* messageFormat, ArgsT&&... args)
    {
        auto formatted_message = std::vformat
        (
            messageFormat,
            std::make_format_args(args...)
        );

        cerr(formatted_message.c_str());
    }

    template <typename... ArgsT>
    void cout(const char* messageFormat, ArgsT&&... args)
    {
        auto formatted_message = std::vformat
        (
            messageFormat,
            std::make_format_args(args...)
        );

        cout(formatted_message.c_str());
    }

} // namespace Logging
