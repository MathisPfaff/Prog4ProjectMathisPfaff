#pragma once

#if USE_STEAMWORKS
#include "Observer.h"
#include "ScoreComponent.h"
#include "SteamAchievements.h"

namespace dae
{
    class SteamAchievementObserver final : public Observer
    {
    public:
        explicit SteamAchievementObserver(ScoreComponent* scoreComp)
            : m_ScoreComp(scoreComp)
        {
            scoreComp->AddObserver(this);
        }

        void OnNotify(BaseComponent*, Event event) override
        {
            if (event == Event::AddScore && !m_Unlocked && m_ScoreComp->GetScore() >= 500)
            {
                m_Unlocked = true;
                SteamAchievements::GetInstance().Unlock(AchievementID::WinOneGame);
            }
        }

    private:
        ScoreComponent* m_ScoreComp;
        bool m_Unlocked{ false };
    };
}
#endif