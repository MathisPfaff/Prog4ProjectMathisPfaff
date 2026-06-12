#pragma once
#include "BaseComponent.h"
#include <glm/glm.hpp>

namespace dae
{
    class HitboxComponent;
    class GridComponent;

    class FireBreathComponent final : public BaseComponent
    {
    public:
        FireBreathComponent(GameObject* owner, GameObject* pGridObject);
        ~FireBreathComponent() override = default;

        void FixedUpdate(float) override {}
        void Update()          override;
        void LateUpdate()      override {}
        void Render()    const override;

        void Fire();
        void StopFire();

        bool IsActive() const { return m_Active; }
        glm::vec2 GetFiringDirection() const { return m_FiringDirection; }

        // Set before calling Fire(); horizontal component is what matters.
        // Fire() enforces horizontal – a vertical direction will be snapped to right.
        void SetFacingDirection(const glm::vec2& dir) { m_FacingDirection = dir; }

    private:
        void UpdateHitbox();

        // Identical contract to PumpComponent::IsSubCellBlocked:
        // returns true when the subcell at grid-relative (relX,relY) blocks the fire
        bool IsFireBlocked(float relX, float relY) const;

        GameObject*      m_pGridObject{};
        HitboxComponent* m_pFireHitbox{};

        bool      m_Active{ false };
        glm::vec2 m_FiringDirection{ 1.f, 0.f };
        glm::vec2 m_FacingDirection{ 1.f, 0.f };
        float     m_CurrentLength{ 0.f };

        enum class FireState { Idle, Extending, Holding, Retracting };
        FireState m_FireState{ FireState::Idle };
        float     m_HoldTimer{ 0.f };

        static constexpr float k_MaxCells    { 3.f   };
        static constexpr float k_ExtendSpeed { 180.f };
        static constexpr float k_RetractSpeed{ 300.f };
        static constexpr float k_HoldTime   { 0.8f  };
    };
}