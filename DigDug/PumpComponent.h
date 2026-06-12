#pragma once
#include "BaseComponent.h"
#include <glm/glm.hpp>

namespace dae
{
    class GridComponent;
    class HitboxComponent;
    class PookaComponent;

    class PumpComponent final : public BaseComponent
    {
    public:
        PumpComponent(GameObject* owner, GameObject* pGridObject);
        ~PumpComponent() override = default;

        void FixedUpdate(float) override {}
        void Update()          override;
        void LateUpdate()      override {}
        void Render()    const override;

        // Pressed binding: starts the beam when idle; no-ops otherwise
        void Fire();

        // Pressed binding (same button): +k_InflatePulse per press when stuck
        void InflatePulse();

        // Held binding (same button): sets flag so UpdateStuck inflates continuously
        void PumpHeld();

        // Any non-pump input (e.g. movement): releases beam, enemy starts deflating
        void ReleaseStuck();

        bool IsActive() const { return m_State != PumpState::Idle; }

    private:
        enum class PumpState { Idle, Extending, Retracting, Stuck };

        void UpdateExtending(float dt);
        void UpdateRetracting(float dt);
        void UpdateStuck(float dt);
        void UpdateHitbox();

        bool IsSubCellBlocked(float relX, float relY) const;

        GameObject*      m_pGridObject{};
        HitboxComponent* m_pHitbox{};
        PookaComponent*  m_pStuckEnemy{};        // non-null only in Stuck state

        PumpState  m_State{ PumpState::Idle };
        glm::vec2  m_FiringDirection{ 1.f, 0.f };
        float      m_CurrentLength{ 0.f };
        bool       m_PumpHeldThisFrame{ false }; // set by PumpHeld(), consumed in UpdateStuck

        static constexpr float k_MaxCells{ 2.f };
        static constexpr float k_ExtendSpeed{ 200.f };
        static constexpr float k_RetractSpeed{ 350.f };
        static constexpr float k_InflatePulse{ 1.5f };    // added per button press  (3 presses = dead)
        static constexpr float k_InflateHeldRate{ 1.5f }; // added per second while held (3s = dead)
    };
}