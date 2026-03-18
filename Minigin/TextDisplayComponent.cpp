#include "TextDisplayComponent.h"
#include "TextComponent.h"
#include "GameObject.h"

dae::TextDisplayComponent::TextDisplayComponent(
	GameObject* owner,
	std::function<std::string()> textGetter,
	std::function<void(Observer*)> subscribeFunc)
	: BaseComponent(owner)
	, m_TextGetter(std::move(textGetter))
{
	if (subscribeFunc)
		subscribeFunc(this); // safe: object is fully constructed at this point
}

void dae::TextDisplayComponent::Update()
{
	if (!m_Dirty) return;
	auto* text = GetOwner()->GetComponent<TextComponent>();
	if (text)
		text->SetText(m_TextGetter());
	m_Dirty = false;
}