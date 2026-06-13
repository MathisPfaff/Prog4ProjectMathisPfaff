#pragma once
#include "BaseComponent.h"
#include <glm/glm.hpp>

namespace dae
{
    class GridComponent;
    class HitboxComponent;
    class PookaComponent;
    class FygarComponent;

    class PumpComponent final : public BaseComponent
    {
    public:
        PumpComponent(GameObject* owner, GameObject* pGridObject);
        ~PumpComponent() override = default;

        void FixedUpdate(float) override {}
        void Update()          override;
        void LateUpdate()      override {}
        void Render()    const override;

        void Fire();
        void InflatePulse();
        void PumpHeld();
        void ReleaseStuck();

        bool IsActive() const { return m_State != PumpState::Idle; }

    private:
        enum class PumpState { Idle, Extending, Retracting, Stuck };

        void UpdateExtending(float dt);
        void UpdateRetracting(float dt);
        void UpdateStuck(float dt);
        void UpdateHitbox();

        bool IsSubCellBlocked(float relX, float relY) const;

        void TryAwardKillScore();

        bool  HasStuckEnemy()           const;
        bool  StuckEnemyAddInflate(float amount);
        void  StuckEnemyStartDeflating();
        void  ClearStuckEnemy();

        GameObject*      m_pGridObject{};
        HitboxComponent* m_pHitbox{};
        PookaComponent*  m_pStuckEnemy     {};
        FygarComponent*  m_pStuckEnemy_Fygar{};

        PumpState  m_State{ PumpState::Idle };
        glm::vec2  m_FiringDirection{ 1.f, 0.f };
        float      m_CurrentLength{ 0.f };
        bool       m_PumpHeldThisFrame{ false };

        bool m_pumpingSoundActive{ false };

        static constexpr float k_MaxCells      { 2.f   };
        static constexpr float k_ExtendSpeed   { 200.f };
        static constexpr float k_RetractSpeed  { 350.f };
        static constexpr float k_InflatePulse  { 1.5f  };
        static constexpr float k_InflateHeldRate{ 1.5f };
    };
}