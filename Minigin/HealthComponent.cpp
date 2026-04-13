#include "HealthComponent.h"
#include <algorithm>

void dae::HealthComponent::FixedUpdate(float deltaTime) { (void)deltaTime; }
void dae::HealthComponent::Update()     {}
void dae::HealthComponent::LateUpdate() {}
void dae::HealthComponent::Render() const {}

void dae::HealthComponent::TakeDamage(int damage)
{
	m_CurrentHealth = std::max(0, m_CurrentHealth - damage);

	m_Subject.Notify(this, HealthEvent::Changed);

	if (m_CurrentHealth <= 0)
		m_Subject.Notify(this, HealthEvent::Died);
}