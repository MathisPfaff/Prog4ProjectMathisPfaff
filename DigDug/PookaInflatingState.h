#pragma once
#include "PookaState.h"

namespace dae
{
    class PookaInflatingState final : public PookaState
    {
    public:
        explicit PookaInflatingState(GameObject* pGridObject);

        void OnEnter(GameObject* owner) override;
        std::unique_ptr<PookaState> Update(GameObject* owner) override;
        void OnExit(GameObject* owner) override;

        // Called by PumpComponent when the player presses any non-pump input
        void Release() { m_Released = true; }

    private:
        GameObject* m_pGridObject{};
        bool        m_Released{ false };
    };
}