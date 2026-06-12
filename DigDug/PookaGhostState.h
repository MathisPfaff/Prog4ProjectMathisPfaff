#pragma once
#include "PookaState.h"
#include <glm/glm.hpp>
#include <string>

namespace dae
{
    class PookaGhostState final : public PookaState
    {
    public:
        explicit PookaGhostState(GameObject* pGridObject,
            float       ghostSpeed   = 35.f,
            std::string ghostTexture = "PookaGhost.png",
            std::string walkTexture  = "Pooka.png");

        void OnEnter(GameObject* owner) override;
        std::unique_ptr<PookaState> Update(GameObject* owner) override;

    private:
        GameObject* m_pGridObject{};
        float       m_GhostSpeed;
        std::string m_GhostTexture;
        std::string m_WalkTexture;

        int         m_TargetCol{ -1 };
        int         m_TargetRow{ -1 };
        glm::vec3   m_TargetWorldPos{};
        glm::vec3   m_MoveDir{};
        bool        m_HasTarget{ false };
    };
}