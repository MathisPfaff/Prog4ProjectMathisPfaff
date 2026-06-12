#pragma once
#include "PookaState.h"

namespace dae
{
    class FireBreathComponent;

    // Fygar stands still, delegates all fire logic to FireBreathComponent,
    // then returns to walking when the flame finishes.
    class FygarBreathingState final : public PookaState
    {
    public:
        explicit FygarBreathingState(GameObject* pGridObject);

        void OnEnter(GameObject* owner) override;
        std::unique_ptr<PookaState> Update(GameObject* owner) override;
        void OnExit(GameObject* owner) override;

    private:
        GameObject*          m_pGridObject{};
        FireBreathComponent* m_pFire{};  // cached on OnEnter – never null after that
    };
}