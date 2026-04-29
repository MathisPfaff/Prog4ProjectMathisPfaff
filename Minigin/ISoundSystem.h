#pragma once
#include <string>

namespace dae
{
    using SoundId = int;

    class ISoundSystem
    {
    public:
        virtual ~ISoundSystem() = default;

        virtual void PlaySound(SoundId id, float volume) = 0;
        virtual void PlayMusic(SoundId id, float volume, bool loop) = 0;
        virtual void StopMusic() = 0;
        virtual void StopAllSounds() = 0;

        virtual void LoadSound(SoundId id, const std::string& filePath) = 0;
        virtual void LoadMusic(SoundId id, const std::string& filePath) = 0;
    };
}