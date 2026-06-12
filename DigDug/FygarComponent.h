#pragma once
#include "BaseComponent.h"
#include "PookaState.h"
#include <memory>

namespace dae
{
    class PookaWalkingState;
    class PookaInflatingState;

    class FygarComponent final : public BaseComponent
    {
    public:
        explicit FygarComponent(GameObject* owner, GameObject* pGridObject = nullptr);

        void FixedUpdate(float) override {}
        void Update() override;
        void LateUpdate() override {}
        void Render() const override {}

        void SetState(std::unique_ptr<PookaState> newState);

        // Same pump contract as PookaComponent
        bool IsPumpable()    const;
        void BeginInflating();
        void StartDeflating();
        bool AddInflate(float amount);

        // Call from a player command in versus mode to trigger fire breath
        void TriggerFireBreath();

        bool IsBreathing() const;

    private:
        std::unique_ptr<PookaState> m_pCurrentState;
        GameObject* m_pGridObject{};
    };
}