#include "Logging.h"

#include <iostream>

namespace Logging
{
    void cerr(const char* message)
    {
        std::cerr << message << std::endl;
    }

    void cout(const char* message)
    {
        std::cout << message << std::endl;
    }

} // namespace Logging
