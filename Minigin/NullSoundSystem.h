#pragma once
#include "ISoundSystem.h"

namespace dae
{
    class NullSoundSystem final : public ISoundSystem
    {
    public:
        void PlaySound(SoundId, float) override {}
        void PlayMusic(SoundId, float, bool) override {}
        void StopMusic() override {}
        void StopAllSounds() override {}

        void LoadSound(SoundId, const std::string&) override {}
        void LoadMusic(SoundId, const std::string&) override {}
    };
}