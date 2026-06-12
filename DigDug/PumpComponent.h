#pragma once
#include "BaseComponent.h"
#include <glm/glm.hpp>

namespace dae
{
    class GridComponent;
    class HitboxComponent;

    class PumpComponent final : public BaseComponent
    {
    public:
        PumpComponent(GameObject* owner, GameObject* pGridObject);
        ~PumpComponent() override = default;

        void FixedUpdate(float) override {}
        void Update()          override;
        void LateUpdate()      override {}
        void Render()    const override;

        // Start a pump cycle if currently idle; called by PumpCommand
        void Fire();

        bool IsActive() const { return m_State != PumpState::Idle; }

    private:
        enum class PumpState { Idle, Extending, Retracting };

        void UpdateExtending(float dt);
        void UpdateRetracting(float dt);
        void UpdateHitbox();

        // Returns true when the subcell at grid-relative (relX, relY) is NOT dug
        // (walls between cells are intentionally NOT checked – pump ignores them)
        bool IsSubCellBlocked(float relX, float relY) const;

        GameObject* m_pGridObject{};
        HitboxComponent* m_pHitbox{};

        PumpState  m_State{ PumpState::Idle };
        glm::vec2  m_FiringDirection{ 1.f, 0.f };       // defaults to right
        float      m_CurrentLength{ 0.f };             // tip distance from player centre (px)

        // Original Dig Dug: pump reaches exactly 2 full grid cells
        static constexpr float k_MaxCells{ 2.f };
        static constexpr float k_ExtendSpeed{ 200.f }; // px/s – feels snappy
        static constexpr float k_RetractSpeed{ 350.f }; // px/s – retracts faster than it extends
    };
}