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

        bool IsPumpable()         const;
        void BeginInflating();
        void StartDeflating();
        bool AddInflate(float amount);

        void ResetToWalking();

    private:
        std::unique_ptr<PookaState> m_pCurrentState;
        GameObject* m_pGridObject{};
    };
}