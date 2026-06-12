#include "FygarBreathingState.h"
#include "PookaWalkingState.h"
#include "FireBreathComponent.h"
#include "TextureComponent.h"
#include "GameObject.h"

namespace dae
{
    FygarBreathingState::FygarBreathingState(GameObject* pGridObject, glm::vec2 fireDirection)
        : m_pGridObject(pGridObject)
        , m_FireDirection(fireDirection)
    {}

    void FygarBreathingState::OnEnter(GameObject* owner)
    {
        if (auto* tex = owner->GetComponent<TextureComponent>())
            tex->SetTexture("Fygar.png");

        m_pFire = owner->GetComponent<FireBreathComponent>();
        if (m_pFire)
        {
            m_pFire->SetFacingDirection(m_FireDirection); // always horizontal, guaranteed by FygarComponent
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