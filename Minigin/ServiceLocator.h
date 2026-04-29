#pragma once
#include <memory>
#include "ISoundSystem.h"
#include "NullSoundSystem.h"

namespace dae
{
    class ServiceLocator final
    {
    public:
        static ISoundSystem& GetSoundSystem() { return *m_pSoundSystem; }

        static void RegisterSoundSystem(std::unique_ptr<ISoundSystem> soundSystem)
        {
            m_pSoundSystem = soundSystem ? std::move(soundSystem) : std::make_unique<NullSoundSystem>();
        }

        ServiceLocator() = delete;
        ~ServiceLocator() = delete;
        ServiceLocator(const ServiceLocator&) = delete;
        ServiceLocator(ServiceLocator&&) = delete;
        ServiceLocator& operator=(const ServiceLocator&) = delete;
        ServiceLocator& operator=(ServiceLocator&&) = delete;

    private:
        static inline std::unique_ptr<ISoundSystem> m_pSoundSystem{ std::make_unique<NullSoundSystem>() };
    };
}