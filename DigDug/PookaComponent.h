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

        // True when pump can latch:
        //   - Walking state (fresh hit)
        //   - Inflating state in Deflating mode (reconnect after release)
        // False for ghost state and actively-inflating state (beam already on enemy)
        bool IsPumpable() const;

        // Walking  -> Inflating (Mode::Inflating), fresh pump hit
        // Deflating mode -> Inflating mode, reconnect (preserves inflate level)
        void BeginInflating();

        // Switch to Deflating mode; beam just released
        void StartDeflating();

        // Add inflate amount; marks the owner for destroy when level reaches max.
        // Returns true if the enemy was killed so the caller can clean up pointers.
        bool AddInflate(float amount);

    private:
        std::unique_ptr<PookaState> m_pCurrentState;
        GameObject* m_pGridObject{};
    };
}