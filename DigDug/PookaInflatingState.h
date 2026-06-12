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

        void UpdateScale(GameObject* owner);
        int GetCurrentStage() const;

        GameObject* m_pGridObject{};
        float       m_InflateLevel{ 0.f };
        int         m_CurrentStage{ 0 };  // tracks which scale stage we're on
        Mode        m_Mode{ Mode::Inflating };

        static constexpr float k_MaxInflate{ 4.5f };
        static constexpr float k_DeflateRate{ 1.0f };   // units/second
        static constexpr float k_BaseScale{ 2.0f };      // normal enemy scale (stage 0)
        static constexpr float k_StageIncrement{ 1.5f }; // inflate level per stage
        
        // Scale per stage: [0]=2.0f, [1]=2.5f, [2]=3.0f, [3]=3.5f
        static constexpr float k_StageScales[4] = { 2.0f, 2.5f, 3.0f, 3.5f };
    };
}