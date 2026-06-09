#include "PookaGhostState.h"
#include "PookaWalkingState.h"
#include "GameObject.h"
#include "TextureComponent.h"
#include "GameTime.h"
#include <cstdlib>

namespace dae
{
    PookaGhostState::PookaGhostState(float ghostSpeed)
        : m_GhostSpeed(ghostSpeed)
    {}

    void PookaGhostState::OnEnter(GameObject* owner)
    {
        if (auto* tex = owner->GetComponent<TextureComponent>())
            tex->SetTexture("PookaGhost.png");

        auto pos = owner->GetWorldPosition();

        float offsetX = static_cast<float>((std::rand() % 201) - 100);
        float offsetY = static_cast<float>((std::rand() % 201) - 100);

        m_Target = pos + glm::vec3(offsetX, offsetY, 0.f);

        glm::vec3 diff = m_Target - pos;
        m_TravelDistance = glm::length(diff);
        m_Direction = m_TravelDistance > 0.f ? glm::normalize(diff) : glm::vec3(1.f, 0.f, 0.f);
        m_Traveled = 0.f;
    }

    std::unique_ptr<PookaState> PookaGhostState::Update(GameObject* owner)
    {
        float step = m_GhostSpeed * GameTime::GetInstance().GetDeltaTime();
        m_Traveled += step;

        auto pos = owner->GetWorldPosition();
        owner->SetLocalPosition(pos + m_Direction * step);

        if (m_Traveled >= m_TravelDistance)
        {
            owner->SetLocalPosition(m_Target);
            return std::make_unique<PookaWalkingState>();
        }

        return nullptr;
    }
}