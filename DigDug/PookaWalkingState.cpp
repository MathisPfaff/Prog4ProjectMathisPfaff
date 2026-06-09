#include "PookaWalkingState.h"
#include "PookaGhostState.h"
#include "GameObject.h"
#include "GameTime.h"
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

    std::unique_ptr<PookaState> PookaWalkingState::Update(GameObject* owner)
    {
        m_Timer += GameTime::GetInstance().GetDeltaTime();

        auto pos = owner->GetWorldPosition();
        owner->SetLocalPosition(
            pos.x + m_Direction.x * m_Speed * GameTime::GetInstance().GetDeltaTime(),
            pos.y + m_Direction.y * m_Speed * GameTime::GetInstance().GetDeltaTime()
        );

        if (m_Timer >= m_WalkDuration)
            return std::make_unique<PookaGhostState>();

        return nullptr;
    }
}