#pragma once
#include "PookaState.h"
#include <glm/glm.hpp>

namespace dae
{
    class PookaWalkingState final : public PookaState
    {
    public:
        explicit PookaWalkingState(float walkDuration = 4.f, float speed = 60.f);

        void OnEnter(GameObject* owner) override;
        void Update(GameObject* owner, float deltaTime) override;

    private:
        float m_WalkDuration;
        float m_Speed;
        float m_Timer{};
        glm::vec2 m_Direction{ 1.f, 0.f };
    };
}