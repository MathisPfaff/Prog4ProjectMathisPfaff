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

        // Start a pump cycle if currently idle; called by PumpCommand
        void Fire();

        // Release a stuck pump; called by any non-pump input command (e.g. MoveCommand)
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
        PookaComponent*  m_pStuckEnemy{};   // non-null while beam is stuck to an enemy

        PumpState  m_State{ PumpState::Idle };
        glm::vec2  m_FiringDirection{ 1.f, 0.f };
        float      m_CurrentLength{ 0.f };

        static constexpr float k_MaxCells{ 2.f };
        static constexpr float k_ExtendSpeed{ 200.f };
        static constexpr float k_RetractSpeed{ 350.f };
    };
}