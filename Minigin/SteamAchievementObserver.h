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
			if (SteamUserStats())
			{
				bool achieved = false;
				SteamUserStats()->GetAchievement(AchievementID::WinOneGame, &achieved);
				m_Unlocked = achieved;
			}

			scoreComp->AddObserver(this);
		}

		void OnNotify(BaseComponent*, unsigned int eventID) override
		{
			if (eventID == ScoreEvent::Added && !m_Unlocked && m_ScoreComp->GetScore() >= 500)
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