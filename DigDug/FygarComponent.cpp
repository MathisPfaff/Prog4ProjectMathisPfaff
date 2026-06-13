#include "FygarComponent.h"
#include "PookaWalkingState.h"
#include "PookaGhostState.h"
#include "PookaInflatingState.h"
#include "FygarBreathingState.h"
#include "GameObject.h"
#include <cstdlib>
#include <cmath>

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

        if (auto* walking = dynamic_cast<PookaWalkingState*>(m_pCurrentState.get()))
        {
            const glm::vec2 dir = walking->GetLastDirection();
            if (std::abs(dir.x) > 0.5f)
                m_LastHorizontalDir = { dir.x > 0.f ? 1.f : -1.f, 0.f };
        }

        auto next = m_pCurrentState->Update(GetOwner());
        if (next)
        {
            if (dynamic_cast<PookaWalkingState*>(m_pCurrentState.get()) &&
                dynamic_cast<PookaGhostState*>(next.get()))
            {
                if (std::rand() % 2 == 0)
                    next = std::make_unique<FygarBreathingState>(m_pGridObject, m_LastHorizontalDir);
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
        if (auto* inf = dynamic_cast<PookaInflatingState*>(m_pCurrentState.get()))
            return inf->IsDeflating();
        return true;
    }

    void FygarComponent::BeginInflating()
    {
        if (auto* inf = dynamic_cast<PookaInflatingState*>(m_pCurrentState.get()))
        { inf->SetInflating(); return; }
        SetState(std::make_unique<PookaInflatingState>(m_pGridObject, 0.f, k_Walk, k_Inflate));
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
            {
                GetOwner()->MarkForDestroy();
                return true;
            }
        }
        return false;
    }

    void FygarComponent::TriggerFireBreath()
    {
        if (!dynamic_cast<PookaWalkingState*>(m_pCurrentState.get())) return;
        SetState(std::make_unique<FygarBreathingState>(m_pGridObject, m_LastHorizontalDir));
    }

    bool FygarComponent::IsBreathing() const
    {
        return dynamic_cast<FygarBreathingState*>(m_pCurrentState.get()) != nullptr;
    }

    void FygarComponent::ResetToWalking()
    {
        SetState(std::make_unique<PookaWalkingState>(m_pGridObject, 10.f, 60.f, k_Walk, k_Ghost));
    }
}