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
        if (dynamic_cast<PookaWalkingState*>(m_pCurrentState.get()))
            return true;

        // Only reconnectable while deflating; actively-inflating enemy
        // already has a beam on it so a second pump must not re-latch.
        if (auto* inflating = dynamic_cast<PookaInflatingState*>(m_pCurrentState.get()))
            return inflating->IsDeflating();

        return false; // ghost state is always immune
    }

    void PookaComponent::BeginInflating()
    {
        // Reconnect: beam re-latches to a deflating enemy -> just flip mode,
        // inflate level is preserved naturally.
        if (auto* inflating = dynamic_cast<PookaInflatingState*>(m_pCurrentState.get()))
        {
            inflating->SetInflating();
            return;
        }

        // Fresh hit from walking state
        if (!dynamic_cast<PookaWalkingState*>(m_pCurrentState.get())) return;
        SetState(std::make_unique<PookaInflatingState>(m_pGridObject));
    }

    void PookaComponent::StartDeflating()
    {
        if (auto* inflating = dynamic_cast<PookaInflatingState*>(m_pCurrentState.get()))
            inflating->SetDeflating();
    }

    bool PookaComponent::AddInflate(float amount)
    {
        if (auto* inflating = dynamic_cast<PookaInflatingState*>(m_pCurrentState.get()))
        {
            if (inflating->AddInflate(amount))
            {
                GetOwner()->MarkForDestroy();
                return true;
            }
        }
        return false;
    }
}