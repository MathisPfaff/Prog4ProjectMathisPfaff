#pragma once
#include "BaseComponent.h"
#include "Subject.h"
#include "Observer.h"
#include "Hash.h"
#include "HealthComponent.h"

namespace ScoreEvent
{
    constexpr unsigned int Added = make_sdbm_hash("ScoreAdded");
}

namespace dae
{
	class ScoreComponent final : public BaseComponent, public Observer
	{
	public:
		explicit ScoreComponent(GameObject* owner, int deathBonus = 500)
			: BaseComponent(owner), m_DeathBonus(deathBonus) {}
		virtual ~ScoreComponent() override = default;
		ScoreComponent(const ScoreComponent&)            = delete;
		ScoreComponent(ScoreComponent&&)                 = delete;
		ScoreComponent& operator=(const ScoreComponent&) = delete;
		ScoreComponent& operator=(ScoreComponent&&)      = delete;

		void FixedUpdate(float) override {}
		void Update()           override {}
		void LateUpdate()       override {}
		void Render() const     override {}

		void OnNotify(BaseComponent*, unsigned int eventID) override
		{
			if (eventID == HealthEvent::Died)
				AddScore(m_DeathBonus);
		}

		void AddScore(int points)
		{
			m_Score += points;
			m_Subject.Notify(this, ScoreEvent::Added);
		}

		int GetScore() const { return m_Score; }

		void AddObserver(Observer* observer)    { m_Subject.AddObserver(observer); }
		void RemoveObserver(Observer* observer) { m_Subject.RemoveObserver(observer); }

	private:
		Subject m_Subject;
		int m_Score{ 0 };
		int m_DeathBonus{ 500 };
	};
}