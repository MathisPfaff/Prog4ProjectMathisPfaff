#include "PookaWalkingState.h"
#include "PookaGhostState.h"
#include "PookaComponent.h"
#include "GameObject.h"
#include "TextureComponent.h"

namespace dae
{
    PookaWalkingState::PookaWalkingState(float walkDuration, float speed)
        : m_WalkDuration(walkDuration), m_Speed(speed)
    {}

    void PookaWalkingState::OnEnter(GameObject* owner)
    {
        if (auto* tex = owner->GetComponent<TextureComponent>())
            tex->SetTexture("Pooka.png");

        m_Timer = 0.f;
        m_Direction.x = -m_Direction.x;
    }

    void PookaWalkingState::Update(GameObject* owner, float deltaTime)
    {
        m_Timer += deltaTime;

        auto pos = owner->GetWorldPosition();
        owner->SetLocalPosition(
            pos.x + m_Direction.x * m_Speed * deltaTime,
            pos.y + m_Direction.y * m_Speed * deltaTime
        );

        if (m_Timer >= m_WalkDuration)
        {
            if (auto* pooka = owner->GetComponent<PookaComponent>())
                pooka->SetState(std::make_unique<PookaGhostState>());
        }
    }
}