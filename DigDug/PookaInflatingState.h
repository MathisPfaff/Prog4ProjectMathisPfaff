#pragma once
#include "PookaState.h"

namespace dae
{
    class PookaInflatingState final : public PookaState
    {
    public:
        explicit PookaInflatingState(GameObject* pGridObject, float inheritedInflateLevel = 0.f);

        void OnEnter(GameObject* owner) override;
        std::unique_ptr<PookaState> Update(GameObject* owner) override;
        void OnExit(GameObject* owner) override;

        // Add to inflate level; returns true when enemy should die (>= 4.5f)
        bool AddInflate(float amount);

        // Called by PumpComponent when beam disconnects / reconnects
        void SetDeflating() { m_Mode = Mode::Deflating; }
        void SetInflating() { m_Mode = Mode::Inflating; }

        bool IsDeflating() const { return m_Mode == Mode::Deflating; }
        float GetInflateLevel() const { return m_InflateLevel; }

    private:
        enum class Mode { Inflating, Deflating };

        GameObject* m_pGridObject{};
        float       m_InflateLevel{ 0.f };
        Mode        m_Mode{ Mode::Inflating };

        static constexpr float k_MaxInflate{ 4.5f };
        static constexpr float k_DeflateRate{ 1.0f }; // units/second (4.5s to fully deflate from max)
    };
}