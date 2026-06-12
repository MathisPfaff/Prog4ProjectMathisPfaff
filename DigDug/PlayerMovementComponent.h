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

        void SetDesiredDirection(const glm::vec2& direction);
        glm::vec2 GetCurrentDirection() const { return m_CurrentDirection; }

    private:
        enum class MoveState
        {
            Idle,             
            AlignToAxis,       
            Moving            
        };

        void UpdateMovement();
        bool NeedsAlignment(const glm::vec2& newDirection) const;
        void StartAlignment(const glm::vec2& newDirection);
        void UpdateMoving(float deltaTime);
        void DigAtCurrentPosition();
        float CalculateSpeed() const;

        GameObject*     m_pGridObject{};
        glm::vec2       m_DesiredDirection{};
        glm::vec2       m_CurrentDirection{};
        MoveState       m_State{ MoveState::Idle };

        glm::vec3       m_AlignmentTarget{};

        static constexpr float m_DiggingSpeed{ 30.f };
        static constexpr float m_WalkingSpeed{ 60.f };
    };
}