#include "Fftw.h"

#include <iostream>
#include <stdexcept>

// TODO:
// Potentially build the library into the program instead.
// Determine library type and implement later. For now, just make console app.
// It's also possible we only need console app.
// Potentially do not throw (just notify) and allow for reading of subsequent
// audio files, if provided.

// For interpreter:
// https://en.wikipedia.org/wiki/Voice_frequency
// (Maybe:)
// constexpr auto VOICE_HZ_MIN = 300;
// constexpr auto VOICE_HZ_MAX = 3400;

// Provide paths to sound files
int main(int argc, char* argv[])
{
    // Test:
    try
    {
        Fftw::analyze("C:\\Dev\\sample-audio-file-human-then-static.raw");
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what();
        return 1;
    }

    // argv[0] = executable path
    /*for (auto i = 1; i < argc; ++i)
    {
        try
        {
            // IMPORTANT: Definitely not this. We need to batch for speed.
            auto analysis = Fftw::analyze(argv[i]);
            std::cout << interpret(analysis); // (Or something)
        }
        catch (const std::exception& ex)
        {
            std::cout << ex.what();
            return 1;
        }
    }*/
}
