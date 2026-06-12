#pragma once
#include "BaseComponent.h"
#include "PookaState.h"
#include <memory>

namespace dae
{
    class PookaComponent final : public BaseComponent
    {
    public:
        explicit PookaComponent(GameObject* owner, GameObject* pGridObject = nullptr);

        void FixedUpdate(float) override {}
        void Update() override;
        void LateUpdate() override {}
        void Render() const override {}

        void SetState(std::unique_ptr<PookaState> newState);

        // True only when in PookaWalkingState – false for ghost and inflating
        bool IsPumpable() const;

        // Transition to PookaInflatingState (no-op if not currently pumpable)
        void BeginInflating();

        // Signal the current PookaInflatingState to release back to walking
        void ReleaseInflating();

    private:
        std::unique_ptr<PookaState> m_pCurrentState;
        GameObject* m_pGridObject{};
    };
}