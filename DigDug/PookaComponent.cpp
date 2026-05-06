#include "PookaComponent.h"
#include "PookaWalkingState.h"
#include "GameObject.h"

namespace dae
{
    PookaComponent::PookaComponent(GameObject* owner)
        : BaseComponent(owner)
    {
        m_pCurrentState = std::make_unique<PookaWalkingState>();
        m_pCurrentState->OnEnter(GetOwner());
    }

    void PookaComponent::Update()
    {
        if (m_pCurrentState)
            m_pCurrentState->Update(GetOwner());
    }

    void PookaComponent::SetState(std::unique_ptr<PookaState> newState)
    {
        if (m_pCurrentState)
            m_pCurrentState->OnExit(GetOwner());

        m_pCurrentState = std::move(newState);

        if (m_pCurrentState)
            m_pCurrentState->OnEnter(GetOwner());
    }
}