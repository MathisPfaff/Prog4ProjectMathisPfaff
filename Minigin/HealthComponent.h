#pragma once
#include "BaseComponent.h"
#include "GameActor.h"

namespace dae
{
	class HealthComponent final : public BaseComponent
	{
	public:
		HealthComponent(GameObject* owner, int maxHealth)
			: BaseComponent(owner), m_GameActor(this), m_MaxHealth(maxHealth), m_CurrentHealth(maxHealth) {}
		virtual ~HealthComponent() override = default;
		HealthComponent(const HealthComponent& other) = delete;
		HealthComponent(HealthComponent&& other) = delete;
		HealthComponent& operator=(const HealthComponent& other) = delete;
		HealthComponent& operator=(HealthComponent&& other) = delete;

		virtual void FixedUpdate(float deltaTime) override;
		virtual void Update() override;
		virtual void LateUpdate() override;
		virtual void Render() const override;

		int GetHealth() const { return m_CurrentHealth; }
		int GetMaxHealth() const { return m_MaxHealth; }
		bool IsDead() const { return m_CurrentHealth <= 0; }

		void TakeDamage(int damage);

		void AddObserver(Observer* observer) { m_GameActor.AddObserver(observer); }
		void RemoveObserver(Observer* observer) { m_GameActor.RemoveObserver(observer); }

	private:
		GameActor m_GameActor;
		int m_MaxHealth{};
		int m_CurrentHealth{};
	};
}