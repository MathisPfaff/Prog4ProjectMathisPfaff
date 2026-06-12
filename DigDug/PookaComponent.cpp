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
        // Only block re-latching while the beam is already actively inflating.
        // Deflating mode is reconnectable; walking and ghost are always pumpable.
        if (auto* inf = dynamic_cast<PookaInflatingState*>(m_pCurrentState.get()))
            return inf->IsDeflating();
        return true;
    }

    void PookaComponent::BeginInflating()
    {
        // Reconnect: beam re-latches to a deflating enemy → flip mode,
        // inflate level is preserved naturally.
        if (auto* inf = dynamic_cast<PookaInflatingState*>(m_pCurrentState.get()))
        {
            inf->SetInflating();
            return;
        }
        // Transition from ANY state (walking, ghost, ...) to inflating.
        SetState(std::make_unique<PookaInflatingState>(m_pGridObject));
    }

    void PookaComponent::StartDeflating()
    {
        if (auto* inf = dynamic_cast<PookaInflatingState*>(m_pCurrentState.get()))
            inf->SetDeflating();
    }

    bool PookaComponent::AddInflate(float amount)
    {
        if (auto* inf = dynamic_cast<PookaInflatingState*>(m_pCurrentState.get()))
        {
            if (inf->AddInflate(amount))
            {
                GetOwner()->MarkForDestroy();
                return true;
            }
        }
        return false;
    }
}