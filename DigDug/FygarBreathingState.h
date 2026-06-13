#pragma once
#include "PookaState.h"
#include <glm/glm.hpp>

namespace dae
{
    class FireBreathComponent;

    class FygarBreathingState final : public PookaState
    {
    public:
        explicit FygarBreathingState(GameObject* pGridObject,
                                     glm::vec2 fireDirection = { 1.f, 0.f });

        void OnEnter(GameObject* owner) override;
        std::unique_ptr<PookaState> Update(GameObject* owner) override;
        void OnExit(GameObject* owner) override;

    private:
        GameObject*          m_pGridObject{};
        FireBreathComponent* m_pFire{};
        glm::vec2            m_FireDirection{ 1.f, 0.f };
    };
}