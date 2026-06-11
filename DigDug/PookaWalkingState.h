#pragma once
#include "PookaState.h"
#include <glm/glm.hpp>

namespace dae
{
    class PookaWalkingState final : public PookaState
    {
    public:
        explicit PookaWalkingState(GameObject* pGridObject,
            float walkDuration = 10.f,
            float speed = 60.f);

        void OnEnter(GameObject* owner) override;
        std::unique_ptr<PookaState> Update(GameObject* owner) override;

    private:
        void PickNextCell(int fromCol, int fromRow, int prevDirX, int prevDirY);

        GameObject* m_pGridObject{};
        float       m_WalkDuration;
        float       m_Speed;
        float       m_Timer{};

        int         m_DirX{ 1 };
        int         m_DirY{ 0 };
        int         m_TargetCol{ -1 };
        int         m_TargetRow{ -1 };
        glm::vec3   m_TargetWorldPos{};
        bool        m_HasTarget{ false };
    };
}