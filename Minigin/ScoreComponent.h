#pragma once
#include "BaseComponent.h"
#include "GameActor.h"
#include "Observer.h"

#if USE_STEAMWORKS
#include "SteamAchievements.h"
#endif

namespace dae
{
	class ScoreComponent final : public BaseComponent, public Observer
	{
	public:
		explicit ScoreComponent(GameObject* owner, int deathBonus = 500)
			: BaseComponent(owner), m_GameActor(this), m_DeathBonus(deathBonus) {}
		virtual ~ScoreComponent() override = default;
		ScoreComponent(const ScoreComponent&) = delete;
		ScoreComponent(ScoreComponent&&) = delete;
		ScoreComponent& operator=(const ScoreComponent&) = delete;
		ScoreComponent& operator=(ScoreComponent&&) = delete;

		void FixedUpdate(float) override {}
		void Update() override {}
		void LateUpdate() override {}
		void Render() const override {}

		// React to opponent dying → award death bonus
		void OnNotify(BaseComponent* /*entity*/, Event event) override
		{
			if (event == Event::PlayerDied)
				AddScore(m_DeathBonus);
		}

		void AddScore(int points)
		{
			m_Score += points;
			m_GameActor.Notify(Event::AddScore);

#if USE_STEAMWORKS
			if (!m_WinAchievementUnlocked && m_Score >= 500)
			{
				m_WinAchievementUnlocked = true;
				SteamAchievements::GetInstance().Unlock(AchievementID::WinOneGame);
			}
#endif
		}

		int GetScore() const { return m_Score; }

		void AddObserver(Observer* observer) { m_GameActor.AddObserver(observer); }
		void RemoveObserver(Observer* observer) { m_GameActor.RemoveObserver(observer); }

	private:
		GameActor m_GameActor;
		int m_Score{ 0 };
		int m_DeathBonus{ 500 };

#if USE_STEAMWORKS
		bool m_WinAchievementUnlocked{ false };
#endif
	};
}