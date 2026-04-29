#pragma once
#include "Observer.h"
#include "ServiceLocator.h"
#include "HealthComponent.h"
#include "ScoreComponent.h"
#include "ISoundSystem.h"

namespace DigDugSounds
{
    constexpr dae::SoundId InGameMusic = 0;
    constexpr dae::SoundId HighScore   = 1;
    constexpr dae::SoundId Miss        = 2;
}

namespace dae
{
    class SoundObserver final : public Observer
    {
    public:
        void OnNotify(BaseComponent* component, unsigned int eventID) override
        {
            if (eventID == HealthEvent::Changed)
            {
                ServiceLocator::GetSoundSystem().PlaySound(DigDugSounds::Miss, 1.f);
            }
            else if (eventID == ScoreEvent::Added && !m_highScorePlayed)
            {
                if (auto* score = dynamic_cast<ScoreComponent*>(component);
                    score && score->GetScore() >= 500)
                {
                    m_highScorePlayed = true;
                    ServiceLocator::GetSoundSystem().StopMusic();
                    ServiceLocator::GetSoundSystem().PlaySound(DigDugSounds::HighScore, 1.f);
                }
            }
        }

    private:
        bool m_highScorePlayed{ false };
    };
}