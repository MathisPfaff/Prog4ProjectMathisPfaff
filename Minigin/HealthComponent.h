#pragma once
#include "BaseComponent.h"
#include "Subject.h"
#include "Hash.h"

namespace HealthEvent
{
    constexpr unsigned int Changed = make_sdbm_hash("HealthChanged");
    constexpr unsigned int Died    = make_sdbm_hash("PlayerDied");
}

namespace dae
{
	class HealthComponent final : public BaseComponent
	{
	public:
		HealthComponent(GameObject* owner, int maxHealth)
			: BaseComponent(owner), m_MaxHealth(maxHealth), m_CurrentHealth(maxHealth) {}
		virtual ~HealthComponent() override = default;
		HealthComponent(const HealthComponent&)            = delete;
		HealthComponent(HealthComponent&&)                 = delete;
		HealthComponent& operator=(const HealthComponent&) = delete;
		HealthComponent& operator=(HealthComponent&&)      = delete;

		virtual void FixedUpdate(float deltaTime) override;
		virtual void Update()       override;
		virtual void LateUpdate()   override;
		virtual void Render() const override;

		int  GetHealth()    const { return m_CurrentHealth; }
		int  GetMaxHealth() const { return m_MaxHealth; }
		bool IsDead()       const { return m_CurrentHealth <= 0; }

		void TakeDamage(int damage);

		void AddObserver(Observer* observer)    { m_Subject.AddObserver(observer); }
		void RemoveObserver(Observer* observer) { m_Subject.RemoveObserver(observer); }

	private:
		Subject m_Subject;
		int m_MaxHealth{};
		int m_CurrentHealth{};
	};
}