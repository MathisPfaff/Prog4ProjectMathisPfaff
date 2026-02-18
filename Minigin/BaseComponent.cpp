#include "BaseComponent.h"

namespace dae
{
	BaseComponent::~BaseComponent() = default;

	void BaseComponent::SetOwner(GameObject* owner)
	{
		m_pOwner = owner;
	}

	GameObject* BaseComponent::GetOwner() const
	{
		return m_pOwner;
	}
}
