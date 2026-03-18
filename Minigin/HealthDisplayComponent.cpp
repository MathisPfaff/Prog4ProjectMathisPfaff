#include "HealthDisplayComponent.h"
#include "HealthComponent.h"
#include "TextComponent.h"
#include "GameObject.h"

dae::HealthDisplayComponent::HealthDisplayComponent(GameObject* owner, HealthComponent* healthComp, const std::string& label)
	: BaseComponent(owner), m_HealthComponent(healthComp), m_Label(label)
{
	if (m_HealthComponent)
		m_HealthComponent->AddObserver(this);
}

void dae::HealthDisplayComponent::Update()
{
	if (!m_isDirty || !m_HealthComponent) return;

	auto* text = GetOwner()->GetComponent<TextComponent>();
	if (text)
		text->SetText(m_Label + std::to_string(m_HealthComponent->GetHealth()));

	m_isDirty = false;
}

void dae::HealthDisplayComponent::OnNotify(BaseComponent* /*entity*/, Event event)
{
	if (event == Event::HealthChanged || event == Event::PlayerDied)
		m_isDirty = true;
}