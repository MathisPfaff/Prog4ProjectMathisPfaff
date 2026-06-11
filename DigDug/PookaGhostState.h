#pragma once
#include "PookaState.h"
#include <glm/glm.hpp>

namespace dae
{
    class PookaGhostState final : public PookaState
    {
    public:
        explicit PookaGhostState(GameObject* pGridObject, float ghostSpeed = 35.f);

        void OnEnter(GameObject* owner) override;
        std::unique_ptr<PookaState> Update(GameObject* owner) override;

    private:
        GameObject* m_pGridObject{};
        float       m_GhostSpeed;

        int         m_TargetCol{ -1 };
        int         m_TargetRow{ -1 };
        glm::vec3   m_TargetWorldPos{};
        glm::vec3   m_MoveDir{};
        bool        m_HasTarget{ false };
    };
}