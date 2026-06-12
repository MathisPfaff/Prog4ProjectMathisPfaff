#pragma once
#include "BaseComponent.h"
#include <glm/glm.hpp>

namespace dae
{
    class HitboxComponent;
    class GridComponent;

    // Fygar's fire-breath weapon.
    // API mirrors PumpComponent so the same Command wrappers work for both.
    //
    //   Fire()     – starts extending the flame (no-op if already active)
    //   StopFire() – immediately extinguishes the flame
    //
    // The owning GameObject needs a HitboxComponent(Enemy) for pump targeting.
    // This component adds its own HitboxComponent(Fire) internally.
    class FireBreathComponent final : public BaseComponent
    {
    public:
        FireBreathComponent(GameObject* owner, GameObject* pGridObject);
        ~FireBreathComponent() override = default;

        void FixedUpdate(float) override {}
        void Update()          override;
        void LateUpdate()      override {}
        void Render()    const override;

        // Start breathing fire in the owner's current facing direction
        void Fire();

        // Stop the flame immediately (called by FygarBreathingState on exit,
        // or by a player-controlled command in versus mode)
        void StopFire();

        bool IsActive() const { return m_Active; }

        // Direction the fire is currently travelling (set on Fire())
        glm::vec2 GetFiringDirection() const { return m_FiringDirection; }

        // Provide the facing direction for AI-driven use
        // (versus mode will drive this through a command instead)
        void SetFacingDirection(const glm::vec2& dir) { m_FacingDirection = dir; }

    private:
        void UpdateHitbox();

        GameObject* m_pGridObject{};
        HitboxComponent* m_pFireHitbox{};  // owned by the same GameObject, type = Fire

        bool      m_Active{ false };
        glm::vec2 m_FiringDirection{ 1.f, 0.f };
        glm::vec2 m_FacingDirection{ 1.f, 0.f }; // last known facing, set each frame by AI
        float     m_CurrentLength{ 0.f };

        // Extend to k_MaxCells cells wide, then hold for k_HoldTime, then retract
        enum class FireState { Idle, Extending, Holding, Retracting };
        FireState m_FireState{ FireState::Idle };
        float     m_HoldTimer{ 0.f };

        static constexpr float k_MaxCells{ 3.f };
        static constexpr float k_ExtendSpeed{ 180.f };
        static constexpr float k_RetractSpeed{ 300.f };
        static constexpr float k_HoldTime{ 0.8f };  // seconds the flame stays at full length
    };
}