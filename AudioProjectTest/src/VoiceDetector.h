#pragma once

// https://en.wikipedia.org/wiki/Voice_frequency
// (Maybe:)
// constexpr auto VOICE_HZ_MIN = 300;
// constexpr auto VOICE_HZ_MAX = 3400;

class VoiceDetector
{
public:
    VoiceDetector();
    virtual ~VoiceDetector() = default;

private:
    // ...

}; // class VoiceDetector
