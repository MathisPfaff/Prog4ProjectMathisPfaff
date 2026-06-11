#pragma once
#include "BaseComponent.h"
#include <glm/glm.hpp>

namespace dae
{
    class GridComponent;

    class PlayerMovementComponent final : public BaseComponent
    {
    public:
        PlayerMovementComponent(GameObject* owner, GameObject* pGridObject);
        ~PlayerMovementComponent() override = default;

        void FixedUpdate(float) override {}
        void Update() override;
        void LateUpdate() override {}
        void Render() const override {}

        // Called by MoveCommand to set desired direction
        void SetDesiredDirection(const glm::vec2& direction);

    private:
        enum class MoveState
        {
            Idle,              // Not moving
            AlignToAxis,       // Aligning to horizontal/vertical center line
            Moving              // Free movement
        };

        void UpdateMovement();
        bool NeedsAlignment(const glm::vec2& newDirection) const;
        void StartAlignment(const glm::vec2& newDirection);
        void UpdateMoving(float deltaTime);
        void DigAtCurrentPosition();          // ✅ NEW
        void CheckAndOpenWalls();          // ✅ NEW
        float CalculateSpeed() const;

        GameObject*     m_pGridObject{};
        glm::vec2       m_DesiredDirection{};   // Set by command each frame
        glm::vec2       m_CurrentDirection{};   // Actual movement direction
        MoveState       m_State{ MoveState::Idle };

        glm::vec3       m_AlignmentTarget{};
        int             m_LastDiggedSubCol{ -1 };
        int             m_LastDiggedSubRow{ -1 };
        int             m_LastWallCheckCol{ -1 };
        int             m_LastWallCheckRow{ -1 };

        static constexpr float m_DiggingSpeed{ 30.f };
        static constexpr float m_WalkingSpeed{ 60.f };
    };
}