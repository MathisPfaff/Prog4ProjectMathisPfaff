#include "PlayerMovementComponent.h"
#include "GameObject.h"
#include "GridComponent.h"
#include "GameTime.h"
#include <glm/glm.hpp>
#include <cmath>

namespace dae
{
    PlayerMovementComponent::PlayerMovementComponent(GameObject* owner, GameObject* pGridObject)
        : BaseComponent(owner)
        , m_pGridObject(pGridObject)
    {}

    void PlayerMovementComponent::Reset()
    {
        m_DesiredDirection = glm::vec2(0.f, 0.f);
        m_CurrentDirection = glm::vec2(0.f, 0.f);
        m_State            = MoveState::Idle;
        m_AlignmentTarget  = glm::vec3(0.f);
    }

    void PlayerMovementComponent::SetDesiredDirection(const glm::vec2& direction)
    {
        m_DesiredDirection = direction;
    }

    void PlayerMovementComponent::Update()
    {
        UpdateMovement();
        m_DesiredDirection = glm::vec2(0.f, 0.f); // Clear after processing
    }

    void PlayerMovementComponent::UpdateMovement()
    {
        auto* grid = m_pGridObject ? m_pGridObject->GetComponent<GridComponent>() : nullptr;
        if (!grid) return;

        auto* owner = GetOwner();
        if (!owner) return;

        const float dt = GameTime::GetInstance().GetDeltaTime();
        const glm::vec3 gridOrigin = m_pGridObject->GetWorldPosition();
        const glm::vec3 currentWorldPos = owner->GetWorldPosition();

        switch (m_State)
        {
        case MoveState::Idle:
        {
            if (glm::length(m_DesiredDirection) < 0.1f) return;

            // Check if we need to align to axis
            if (NeedsAlignment(m_DesiredDirection))
            {
                StartAlignment(m_DesiredDirection);
                m_State = MoveState::AlignToAxis;
            }
            else
            {
                // Start moving immediately
                m_CurrentDirection = m_DesiredDirection;
                m_State = MoveState::Moving;
            }
            break;
        }

        case MoveState::AlignToAxis:
        {
            // Move toward alignment target
            const glm::vec3 toTarget = m_AlignmentTarget - currentWorldPos;
            const float dist = glm::length(toTarget);
            const float step = m_WalkingSpeed * dt;

            if (step >= dist || dist < 0.5f)
            {
                // Aligned
                owner->SetLocalPosition(m_AlignmentTarget);
                m_CurrentDirection = m_DesiredDirection;
                m_State = MoveState::Moving;
            }
            else
            {
                owner->SetLocalPosition(currentWorldPos + (toTarget / dist) * step);
                DigAtCurrentPosition();
            }
            break;
        }

        case MoveState::Moving:
        {
            // Check if direction changed and needs alignment
            if (glm::length(m_DesiredDirection) > 0.1f && NeedsAlignment(m_DesiredDirection))
            {
                StartAlignment(m_DesiredDirection);
                m_State = MoveState::AlignToAxis;
                return;
            }

            // Check if player stopped (no input)
            if (glm::length(m_DesiredDirection) < 0.1f)
            {
                m_State = MoveState::Idle;
                return;
            }

            // Update direction if still same axis
            if (glm::length(m_DesiredDirection) > 0.1f)
            {
                m_CurrentDirection = m_DesiredDirection;
            }

            UpdateMoving(dt);
            break;
        }
        }
    }

    bool PlayerMovementComponent::NeedsAlignment(const glm::vec2& newDirection) const
    {
        if (glm::length(m_CurrentDirection) < 0.1f)
        {
            // First movement - no alignment needed
            return false;
        }

        // Check if changing from horizontal to vertical or vice versa
        const bool currentIsHorizontal = std::abs(m_CurrentDirection.x) > std::abs(m_CurrentDirection.y);
        const bool newIsHorizontal = std::abs(newDirection.x) > std::abs(newDirection.y);

        return currentIsHorizontal != newIsHorizontal;
    }

    void PlayerMovementComponent::StartAlignment(const glm::vec2& newDirection)
    {
        auto* grid = m_pGridObject->GetComponent<GridComponent>();
        if (!grid) return;

        auto* owner = GetOwner();
        if (!owner) return;

        const glm::vec3 gridOrigin = m_pGridObject->GetWorldPosition();
        const glm::vec3 currentWorldPos = owner->GetWorldPosition();
        const float relX = currentWorldPos.x - gridOrigin.x;
        const float relY = currentWorldPos.y - gridOrigin.y;

        // Determine which axis to align to
        const bool newIsHorizontal = std::abs(newDirection.x) > std::abs(newDirection.y);

        if (newIsHorizontal)
        {
            // Need to align to horizontal center line (middle of vertical range)
            int subCol{}, subRow{};
            grid->WorldToSubCell(relX, relY, subCol, subRow);
            
            // Find the row's horizontal center line (subRow % 3 == 1)
            const int row = subRow / 3;
            const int centerSubRow = row * 3 + 1;
            
            float _, centerY;
            grid->SubCellToWorld(subCol, centerSubRow, _, centerY);
            m_AlignmentTarget = glm::vec3(currentWorldPos.x, gridOrigin.y + centerY, currentWorldPos.z);
        }
        else
        {
            // Need to align to vertical center line (middle of horizontal range)
            int subCol{}, subRow{};
            grid->WorldToSubCell(relX, relY, subCol, subRow);
            
            // Find the column's vertical center line (subCol % 3 == 1)
            const int col = subCol / 3;
            const int centerSubCol = col * 3 + 1;
            
            float centerX, _;
            grid->SubCellToWorld(centerSubCol, subRow, centerX, _);
            m_AlignmentTarget = glm::vec3(gridOrigin.x + centerX, currentWorldPos.y, currentWorldPos.z);
        }
    }

    void PlayerMovementComponent::DigAtCurrentPosition()
    {
        auto* grid = m_pGridObject->GetComponent<GridComponent>();
        if (!grid) return;

        auto* owner = GetOwner();
        if (!owner) return;

        const glm::vec3 gridOrigin = m_pGridObject->GetWorldPosition();
        const glm::vec3 worldPos = owner->GetWorldPosition();
        const float relX = worldPos.x - gridOrigin.x;
        const float relY = worldPos.y - gridOrigin.y;

        const int centerSubCol = static_cast<int>(std::floor(relX / grid->GetSubCellWidth()));
        const int centerSubRow = static_cast<int>(std::floor(relY / grid->GetSubCellHeight()));

        // Dig the stable 3x3
        for (int dr = -1; dr <= 1; ++dr)
            for (int dc = -1; dc <= 1; ++dc)
                grid->DigSubCell(centerSubCol + dc, centerSubRow + dr);

        // Open the wall between two big-cells only when the player is the one
        // crossing — and only while their 3x3 actually straddles that boundary.
        // OpenSide uses |= so calling it every frame while straddling is safe.
        const int centerCol = centerSubCol / 3;
        const int centerRow = centerSubRow / 3;

        const bool movingHorizontal = std::abs(m_CurrentDirection.x) > std::abs(m_CurrentDirection.y);
        if (movingHorizontal)
        {
            const int leadingSubCol = centerSubCol + (m_CurrentDirection.x > 0.f ? 1 : -1);
            const int leadingCol = leadingSubCol / 3;
            if (leadingCol != centerCol)   // 3x3 straddles the vertical boundary
            {
                if (m_CurrentDirection.x > 0.f)
                {
                    grid->OpenSide(centerCol, centerRow, TunnelSide::Right);
                    grid->OpenSide(leadingCol, centerRow, TunnelSide::Left);
                }
                else
                {
                    grid->OpenSide(centerCol, centerRow, TunnelSide::Left);
                    grid->OpenSide(leadingCol, centerRow, TunnelSide::Right);
                }
            }
        }
        else
        {
            const int leadingSubRow = centerSubRow + (m_CurrentDirection.y > 0.f ? 1 : -1);
            const int leadingRow = leadingSubRow / 3;
            if (leadingRow != centerRow)   // 3x3 straddles the horizontal boundary
            {
                if (m_CurrentDirection.y > 0.f)
                {
                    grid->OpenSide(centerCol, centerRow, TunnelSide::Down);
                    grid->OpenSide(centerCol, leadingRow, TunnelSide::Up);
                }
                else
                {
                    grid->OpenSide(centerCol, centerRow, TunnelSide::Up);
                    grid->OpenSide(centerCol, leadingRow, TunnelSide::Down);
                }
            }
        }
    }

    void PlayerMovementComponent::UpdateMoving(float deltaTime)
    {
        auto* grid = m_pGridObject->GetComponent<GridComponent>();
        if (!grid) return;

        auto* owner = GetOwner();
        if (!owner) return;

        const glm::vec3 gridOrigin = m_pGridObject->GetWorldPosition();
        const glm::vec3 currentWorldPos = owner->GetWorldPosition();

        const float speed = CalculateSpeed();
        const glm::vec3 moveDir = glm::vec3(m_CurrentDirection.x, m_CurrentDirection.y, 0.f);
        const glm::vec3 newPos = currentWorldPos + moveDir * speed * deltaTime;

        const float relX = newPos.x - gridOrigin.x;
        const float relY = newPos.y - gridOrigin.y;

        const float halfW = 1.5f * grid->GetSubCellWidth();
        const float halfH = 1.5f * grid->GetSubCellHeight();
        if (relX - halfW < 0.f || relX + halfW > grid->GetTotalWidth() ||
            relY - halfH < 0.f || relY + halfH > grid->GetTotalHeight())
        {
            m_State = MoveState::Idle;
            return;
        }

        owner->SetLocalPosition(newPos);
        DigAtCurrentPosition();   // walls now open automatically inside DigSubCell
    }

    float PlayerMovementComponent::CalculateSpeed() const
    {
        auto* grid = m_pGridObject->GetComponent<GridComponent>();
        if (!grid) return m_DiggingSpeed;

        auto* owner = GetOwner();
        if (!owner) return m_DiggingSpeed;

        if (glm::length(m_CurrentDirection) < 0.1f) return m_WalkingSpeed;

        const glm::vec3 gridOrigin = m_pGridObject->GetWorldPosition();
        const glm::vec3 worldPos = owner->GetWorldPosition();
        const float relX = worldPos.x - gridOrigin.x;
        const float relY = worldPos.y - gridOrigin.y;

        const float halfW = 1.5f * grid->GetSubCellWidth();
        const float halfH = 1.5f * grid->GetSubCellHeight();

        // Digging speed if ANY overlapping subcell is not yet dug, walking speed otherwise
        return grid->IsBoxTouchingUndugSubCell(relX - halfW, relY - halfH, relX + halfW, relY + halfH)
            ? m_DiggingSpeed : m_WalkingSpeed;
    }
}