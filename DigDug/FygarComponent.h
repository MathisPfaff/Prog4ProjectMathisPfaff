#pragma once
#include "BaseComponent.h"
#include "PookaState.h"
#include <glm/glm.hpp>
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

        bool IsPumpable()    const;
        void BeginInflating();
        void StartDeflating();
        bool AddInflate(float amount);
        void TriggerFireBreath();
        bool IsBreathing() const;

        void ResetToWalking();

    private:
        std::unique_ptr<PookaState> m_pCurrentState;
        GameObject* m_pGridObject{};
        glm::vec2   m_LastHorizontalDir{ 1.f, 0.f };
    };
}