#pragma once
#include "PookaState.h"
#include <glm/glm.hpp>

namespace dae
{
    class FireBreathComponent;

    // Fygar stands still, delegates all fire logic to FireBreathComponent,
    // then returns to walking when the flame finishes.
    class FygarBreathingState final : public PookaState
    {
    public:
        // fireDirection must be horizontal ({±1,0}); enforced again inside FireBreathComponent::Fire()
        explicit FygarBreathingState(GameObject* pGridObject,
                                     glm::vec2 fireDirection = { 1.f, 0.f });

        void OnEnter(GameObject* owner) override;
        std::unique_ptr<PookaState> Update(GameObject* owner) override;
        void OnExit(GameObject* owner) override;

    private:
        GameObject*          m_pGridObject{};
        FireBreathComponent* m_pFire{};  // cached on OnEnter – never null after that
        glm::vec2            m_FireDirection{ 1.f, 0.f };
    };
}