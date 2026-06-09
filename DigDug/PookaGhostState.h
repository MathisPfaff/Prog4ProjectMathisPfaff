#pragma once
#include "PookaState.h"
#include <glm/glm.hpp>

namespace dae
{
    class PookaGhostState final : public PookaState
    {
    public:
        explicit PookaGhostState(float ghostSpeed = 100.f);

        void OnEnter(GameObject* owner) override;
        std::unique_ptr<PookaState> Update(GameObject* owner) override;

    private:
        float m_GhostSpeed;
        glm::vec3 m_Target{};
        glm::vec3 m_Direction{};
        float m_TravelDistance{};
        float m_Traveled{};
    };
}