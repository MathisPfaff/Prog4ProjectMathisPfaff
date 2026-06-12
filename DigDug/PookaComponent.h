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

        // Returns false only while the beam is actively inflating the enemy
        // (prevents a second pump latching on at the same time).
        // Walking, ghost, and deflating-inflate states are all pumpable.
        bool IsPumpable() const;

        // Any state -> Inflating (Mode::Inflating), or
        // Deflating mode -> Inflating mode (reconnect, preserves inflate level).
        void BeginInflating();

        // Switch to Deflating mode; beam just released.
        void StartDeflating();

        // Add inflate amount; marks the owner for destroy when level reaches max.
        // Returns true if the enemy was killed so the caller can clean up pointers.
        bool AddInflate(float amount);

    private:
        std::unique_ptr<PookaState> m_pCurrentState;
        GameObject* m_pGridObject{};
    };
}