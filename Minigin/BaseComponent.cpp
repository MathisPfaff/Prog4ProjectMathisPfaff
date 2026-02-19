#include "BaseComponent.h"

namespace dae
{
	BaseComponent::~BaseComponent() = default;

	GameObject* BaseComponent::GetOwner() const
	{
		return m_pOwner;
	}
}
