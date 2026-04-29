#pragma once
#include "ISoundSystem.h"
#include <memory>

namespace dae
{
    class SDLSoundSystem final : public ISoundSystem
    {
    public:
        SDLSoundSystem();
        ~SDLSoundSystem() override;

        SDLSoundSystem(const SDLSoundSystem&) = delete;
        SDLSoundSystem(SDLSoundSystem&&) = delete;
        SDLSoundSystem& operator=(const SDLSoundSystem&) = delete;
        SDLSoundSystem& operator=(SDLSoundSystem&&) = delete;

        void PlaySound(SoundId id, float volume) override;
        void PlayMusic(SoundId id, float volume, bool loop) override;
        void StopMusic() override;
        void StopAllSounds() override;

        void LoadSound(SoundId id, const std::string& filePath) override;
        void LoadMusic(SoundId id, const std::string& filePath) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}