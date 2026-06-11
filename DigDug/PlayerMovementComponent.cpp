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

    void PlayerMovementComponent::UpdateMoving(float deltaTime)
    {
        auto* grid = m_pGridObject->GetComponent<GridComponent>();
        if (!grid) return;

        auto* owner = GetOwner();
        if (!owner) return;

        const glm::vec3 gridOrigin = m_pGridObject->GetWorldPosition();
        const glm::vec3 currentWorldPos = owner->GetWorldPosition();

        // Calculate movement
        const float speed = CalculateSpeed();
        const glm::vec3 moveDir = glm::vec3(m_CurrentDirection.x, m_CurrentDirection.y, 0.f);
        const glm::vec3 newPos = currentWorldPos + moveDir * speed * deltaTime;

        // Check bounds
        const float relX = newPos.x - gridOrigin.x;
        const float relY = newPos.y - gridOrigin.y;
        int subCol{}, subRow{};
        
        if (!grid->WorldToSubCell(relX, relY, subCol, subRow))
        {
            // Out of bounds, stop
            m_State = MoveState::Idle;
            return;
        }

        const int subColMax = grid->GetSubCols() - 2;
        const int subRowMax = grid->GetSubRows() - 2;
        if (subCol < 1 || subCol > subColMax || subRow < 1 || subRow > subRowMax)
        {
            m_State = MoveState::Idle;
            return;
        }

        // Move
        owner->SetLocalPosition(newPos);

        // Dig continuously
        DigAtCurrentPosition();

        // Check walls
        CheckAndOpenWalls();
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

        // Get the CENTER subcell of the character's 3x3 footprint
        int centerSubCol{}, centerSubRow{};
        if (!grid->WorldToSubCell(relX, relY, centerSubCol, centerSubRow)) return;

        // Only dig if we're in a new subcell
        if (centerSubCol == m_LastDiggedSubCol && centerSubRow == m_LastDiggedSubRow) return;

        m_LastDiggedSubCol = centerSubCol;
        m_LastDiggedSubRow = centerSubRow;

        // Character occupies exactly 3x3 subcells centered on centerSubCol, centerSubRow
        // Dig all 9 subcells that the character covers
        grid->DigSubCell(centerSubCol - 1, centerSubRow - 1); // Top-left
        grid->DigSubCell(centerSubCol, centerSubRow - 1); // Top-center
        grid->DigSubCell(centerSubCol + 1, centerSubRow - 1); // Top-right

        grid->DigSubCell(centerSubCol - 1, centerSubRow);     // Middle-left
        grid->DigSubCell(centerSubCol, centerSubRow);     // Middle-center (character center)
        grid->DigSubCell(centerSubCol + 1, centerSubRow);     // Middle-right

        grid->DigSubCell(centerSubCol - 1, centerSubRow + 1); // Bottom-left
        grid->DigSubCell(centerSubCol, centerSubRow + 1); // Bottom-center
        grid->DigSubCell(centerSubCol + 1, centerSubRow + 1); // Bottom-right

        // Check if we need to open walls when crossing cell boundaries
        const int currentCol = centerSubCol / 3;
        const int currentRow = centerSubRow / 3;

        const bool movingHorizontal = std::abs(m_CurrentDirection.x) > std::abs(m_CurrentDirection.y);

        if (movingHorizontal)
        {
            // Check the edge of the 3x3 in the direction of movement
            const int edgeSubCol = centerSubCol + (m_CurrentDirection.x > 0 ? 1 : -1);
            const int edgeCol = edgeSubCol / 3;

            if (edgeCol != currentCol)
            {
                if (m_CurrentDirection.x > 0)
                {
                    grid->OpenSide(currentCol, currentRow, TunnelSide::Right);
                    grid->OpenSide(edgeCol, currentRow, TunnelSide::Left);
                }
                else
                {
                    grid->OpenSide(currentCol, currentRow, TunnelSide::Left);
                    grid->OpenSide(edgeCol, currentRow, TunnelSide::Right);
                }
            }
        }
        else
        {
            // Check the edge of the 3x3 in the direction of movement
            const int edgeSubRow = centerSubRow + (m_CurrentDirection.y > 0 ? 1 : -1);
            const int edgeRow = edgeSubRow / 3;

            if (edgeRow != currentRow)
            {
                if (m_CurrentDirection.y > 0)
                {
                    grid->OpenSide(currentCol, currentRow, TunnelSide::Down);
                    grid->OpenSide(currentCol, edgeRow, TunnelSide::Up);
                }
                else
                {
                    grid->OpenSide(currentCol, currentRow, TunnelSide::Up);
                    grid->OpenSide(currentCol, edgeRow, TunnelSide::Down);
                }
            }
        }
    }

    void PlayerMovementComponent::CheckAndOpenWalls()
    {
        auto* grid = m_pGridObject->GetComponent<GridComponent>();
        if (!grid) return;

        auto* owner = GetOwner();
        if (!owner) return;

        const glm::vec3 gridOrigin = m_pGridObject->GetWorldPosition();
        const glm::vec3 worldPos = owner->GetWorldPosition();
        const float relX = worldPos.x - gridOrigin.x;
        const float relY = worldPos.y - gridOrigin.y;

        int subCol{}, subRow{};
        if (!grid->WorldToSubCell(relX, relY, subCol, subRow)) return;

        const int col = subCol / 3;
        const int row = subRow / 3;

        // Only check walls when entering a new cell
        if (col == m_LastWallCheckCol && row == m_LastWallCheckRow) return;

        const int prevCol = m_LastWallCheckCol;
        const int prevRow = m_LastWallCheckRow;

        m_LastWallCheckCol = col;
        m_LastWallCheckRow = row;

        // First movement - no walls to open yet
        if (prevCol == -1 || prevRow == -1) return;

        // Determine which wall to open based on cell change
        TunnelSide exitSide = TunnelSide::None;
        TunnelSide entrySide = TunnelSide::None;

        if (col > prevCol)      { exitSide = TunnelSide::Right; entrySide = TunnelSide::Left; }
        else if (col < prevCol) { exitSide = TunnelSide::Left;  entrySide = TunnelSide::Right; }
        else if (row > prevRow) { exitSide = TunnelSide::Down;  entrySide = TunnelSide::Up; }
        else if (row < prevRow) { exitSide = TunnelSide::Up;    entrySide = TunnelSide::Down; }

        if (exitSide != TunnelSide::None)
        {
            grid->OpenSide(prevCol, prevRow, exitSide);
            grid->OpenSide(col, row, entrySide);
        }
    }

    float PlayerMovementComponent::CalculateSpeed() const
    {
        auto* grid = m_pGridObject->GetComponent<GridComponent>();
        if (!grid) return m_DiggingSpeed;

        auto* owner = GetOwner();
        if (!owner) return m_DiggingSpeed;

        // If not moving, use walking speed
        if (glm::length(m_CurrentDirection) < 0.1f) return m_WalkingSpeed;

        const glm::vec3 gridOrigin = m_pGridObject->GetWorldPosition();
        const glm::vec3 worldPos = owner->GetWorldPosition();
        const float relX = worldPos.x - gridOrigin.x;
        const float relY = worldPos.y - gridOrigin.y;

        int subCol{}, subRow{};
        if (!grid->WorldToSubCell(relX, relY, subCol, subRow)) return m_DiggingSpeed;

        // Check the 3 subcells on the edge of the 3x3 area in the direction of movement
        const bool movingHorizontal = std::abs(m_CurrentDirection.x) > std::abs(m_CurrentDirection.y);

        if (movingHorizontal)
        {
            // Check the left or right column of the 3x3
            const int edgeSubCol = subCol + (m_CurrentDirection.x > 0 ? 1 : -1);

            // Check all 3 subcells on that edge
            const bool allDug = grid->IsSubCellDug(edgeSubCol, subRow - 1) &&
                grid->IsSubCellDug(edgeSubCol, subRow) &&
                grid->IsSubCellDug(edgeSubCol, subRow + 1);

            return allDug ? m_WalkingSpeed : m_DiggingSpeed;
        }
        else
        {
            // Check the top or bottom row of the 3x3
            const int edgeSubRow = subRow + (m_CurrentDirection.y > 0 ? 1 : -1);

            // Check all 3 subcells on that edge
            const bool allDug = grid->IsSubCellDug(subCol - 1, edgeSubRow) &&
                grid->IsSubCellDug(subCol, edgeSubRow) &&
                grid->IsSubCellDug(subCol + 1, edgeSubRow);

            return allDug ? m_WalkingSpeed : m_DiggingSpeed;
        }
    }
}