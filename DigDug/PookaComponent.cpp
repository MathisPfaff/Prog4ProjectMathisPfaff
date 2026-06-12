#include "PookaComponent.h"
#include "PookaWalkingState.h"
#include "PookaInflatingState.h"
#include "GameObject.h"

namespace dae
{
    PookaComponent::PookaComponent(GameObject* owner, GameObject* pGridObject)
        : BaseComponent(owner)
        , m_pGridObject(pGridObject)
    {
        m_pCurrentState = std::make_unique<PookaWalkingState>(pGridObject);
        m_pCurrentState->OnEnter(GetOwner());
    }

    void PookaComponent::Update()
    {
        if (m_pCurrentState)
        {
            auto nextState = m_pCurrentState->Update(GetOwner());
            if (nextState)
                SetState(std::move(nextState));
        }
    }

    void PookaComponent::SetState(std::unique_ptr<PookaState> newState)
    {
        if (m_pCurrentState)
            m_pCurrentState->OnExit(GetOwner());

        m_pCurrentState = std::move(newState);

        if (m_pCurrentState)
            m_pCurrentState->OnEnter(GetOwner());
    }

    bool PookaComponent::IsPumpable() const
    {
        // Only a Pooka walking tunnels can be hit by the pump;
        // ghost and already-inflating states are immune.
        return dynamic_cast<PookaWalkingState*>(m_pCurrentState.get()) != nullptr;
    }

    void PookaComponent::BeginInflating()
    {
        if (!IsPumpable()) return;
        SetState(std::make_unique<PookaInflatingState>(m_pGridObject));
    }

    void PookaComponent::ReleaseInflating()
    {
        if (auto* inflating = dynamic_cast<PookaInflatingState*>(m_pCurrentState.get()))
            inflating->Release();
    }
}