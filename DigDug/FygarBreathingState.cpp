#include "FygarBreathingState.h"
#include "PookaWalkingState.h"
#include "FireBreathComponent.h"
#include "TextureComponent.h"
#include "GameObject.h"

namespace dae
{
    FygarBreathingState::FygarBreathingState(GameObject* pGridObject)
        : m_pGridObject(pGridObject)
    {}

    void FygarBreathingState::OnEnter(GameObject* owner)
    {
        if (auto* tex = owner->GetComponent<TextureComponent>())
            tex->SetTexture("Fygar.png"); // reuse walk sprite until you have a fire sprite

        m_pFire = owner->GetComponent<FireBreathComponent>();
        if (m_pFire)
        {
            // Default facing right; replace with actual last-walk direction when AI is complete
            m_pFire->SetFacingDirection({ 1.f, 0.f });
            m_pFire->Fire();
        }
    }

    std::unique_ptr<PookaState> FygarBreathingState::Update(GameObject* /*owner*/)
    {
        if (m_pFire && !m_pFire->IsActive())
            return std::make_unique<PookaWalkingState>(
                m_pGridObject, 10.f, 60.f, "Fygar.png", "FygarGhost.png");
        return nullptr;
    }

    void FygarBreathingState::OnExit(GameObject* owner)
    {
        if (m_pFire && m_pFire->IsActive())
            m_pFire->StopFire();

        if (auto* tex = owner->GetComponent<TextureComponent>())
            tex->SetTexture("Fygar.png");
    }
}