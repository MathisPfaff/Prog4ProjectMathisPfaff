#include "DamageCommand.h"
#include "HealthComponent.h"

dae::DamageCommand::DamageCommand(HealthComponent* healthComp, int damage)
	: m_HealthComponent(healthComp), m_Damage(damage)
{}

void dae::DamageCommand::Execute()
{
	if (m_HealthComponent && !m_HealthComponent->IsDead())
		m_HealthComponent->TakeDamage(m_Damage);
}