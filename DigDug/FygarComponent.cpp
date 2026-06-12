#include "FygarComponent.h"
#include "PookaWalkingState.h"
#include "PookaGhostState.h"
#include "PookaInflatingState.h"
#include "FygarBreathingState.h"
#include "GameObject.h"
#include <cstdlib>

namespace dae
{
    static constexpr const char* k_Walk    = "Fygar.png";
    static constexpr const char* k_Ghost   = "FygarGhost.png";
    static constexpr const char* k_Inflate = "FygarInflateStage0.png";

    FygarComponent::FygarComponent(GameObject* owner, GameObject* pGridObject)
        : BaseComponent(owner)
        , m_pGridObject(pGridObject)
    {
        m_pCurrentState = std::make_unique<PookaWalkingState>(
            pGridObject, 10.f, 60.f, k_Walk, k_Ghost);
        m_pCurrentState->OnEnter(GetOwner());
    }

    void FygarComponent::Update()
    {
        if (!m_pCurrentState) return;

        auto next = m_pCurrentState->Update(GetOwner());
        if (next)
        {
            // When PookaWalkingState's timer expires it always returns a PookaGhostState.
            // Fygar intercepts that exact transition and randomly picks ghost OR fire breath.
            // Everything else (ghost→walk, breath→walk, pump hits) passes straight through.
            if (dynamic_cast<PookaWalkingState*>(m_pCurrentState.get()) &&
                dynamic_cast<PookaGhostState*>(next.get()))
            {
                if (std::rand() % 2 == 0)
                    next = std::make_unique<FygarBreathingState>(m_pGridObject);
                // else: keep next as PookaGhostState — ghost behaviour identical to Pooka
            }

            SetState(std::move(next));
        }
    }

    void FygarComponent::SetState(std::unique_ptr<PookaState> newState)
    {
        if (m_pCurrentState) m_pCurrentState->OnExit(GetOwner());
        m_pCurrentState = std::move(newState);
        if (m_pCurrentState) m_pCurrentState->OnEnter(GetOwner());
    }

    bool FygarComponent::IsPumpable() const
    {
        if (dynamic_cast<PookaWalkingState*>(m_pCurrentState.get()))    return true;
        if (auto* inf = dynamic_cast<PookaInflatingState*>(m_pCurrentState.get()))
            return inf->IsDeflating();
        return false; // ghost and breathing are immune
    }

    void FygarComponent::BeginInflating()
    {
        if (auto* inf = dynamic_cast<PookaInflatingState*>(m_pCurrentState.get()))
        { inf->SetInflating(); return; }

        if (!dynamic_cast<PookaWalkingState*>(m_pCurrentState.get())) return;
        SetState(std::make_unique<PookaInflatingState>(
            m_pGridObject, 0.f, k_Walk, k_Inflate));
    }

    void FygarComponent::StartDeflating()
    {
        if (auto* inf = dynamic_cast<PookaInflatingState*>(m_pCurrentState.get()))
            inf->SetDeflating();
    }

    bool FygarComponent::AddInflate(float amount)
    {
        if (auto* inf = dynamic_cast<PookaInflatingState*>(m_pCurrentState.get()))
        {
            if (inf->AddInflate(amount))
            { GetOwner()->MarkForDestroy(); return true; }
        }
        return false;
    }

    void FygarComponent::TriggerFireBreath()
    {
        if (!dynamic_cast<PookaWalkingState*>(m_pCurrentState.get())) return;
        SetState(std::make_unique<FygarBreathingState>(m_pGridObject));
    }

    bool FygarComponent::IsBreathing() const
    {
        return dynamic_cast<FygarBreathingState*>(m_pCurrentState.get()) != nullptr;
    }
}