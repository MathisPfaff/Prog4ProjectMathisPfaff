#pragma once
#include "PookaState.h"
#include <string>

namespace dae
{
    class PookaInflatingState final : public PookaState
    {
    public:
        explicit PookaInflatingState(GameObject* pGridObject,
            float       inheritedInflateLevel = 0.f,
            std::string walkTexture           = "Pooka.png",
            std::string inflateTexture        = "PookaInflateStage0.png");

        void OnEnter(GameObject* owner) override;
        std::unique_ptr<PookaState> Update(GameObject* owner) override;
        void OnExit(GameObject* owner) override;

        bool AddInflate(float amount);

        void SetDeflating() { m_Mode = Mode::Deflating; }
        void SetInflating() { m_Mode = Mode::Inflating; }

        bool IsDeflating()      const { return m_Mode == Mode::Deflating; }
        float GetInflateLevel() const { return m_InflateLevel; }

    private:
        enum class Mode { Inflating, Deflating };

        void UpdateScale(GameObject* owner);
        int  GetCurrentStage() const;

        GameObject* m_pGridObject{};
        float       m_InflateLevel{ 0.f };
        int         m_CurrentStage{ 0 };
        Mode        m_Mode{ Mode::Inflating };
        std::string m_WalkTexture;
        std::string m_InflateTexture;

        static constexpr float k_MaxInflate    { 4.5f };
        static constexpr float k_DeflateRate   { 1.0f };
        static constexpr float k_BaseScale     { 2.0f };
        static constexpr float k_StageIncrement{ 1.5f };
        static constexpr float k_StageScales[4]{ 2.0f, 2.5f, 3.0f, 3.5f };
    };
}