#include "HealthComponent.h"
#include <algorithm>

void dae::HealthComponent::FixedUpdate(float) {}
void dae::HealthComponent::Update()     {}
void dae::HealthComponent::LateUpdate() {}
void dae::HealthComponent::Render() const {}

void dae::HealthComponent::TakeDamage(int damage)
{
    if (m_CurrentHealth <= 0) return;

    m_CurrentHealth -= damage;
    if (m_CurrentHealth < 0) m_CurrentHealth = 0;

    m_Subject.Notify(this, HealthEvent::Changed);

    if (m_CurrentHealth == 0)
    {
        m_Subject.Notify(this, HealthEvent::Died);
    }
}