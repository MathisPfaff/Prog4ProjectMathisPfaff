#include "PookaGhostState.h"
#include "PookaWalkingState.h"
#include "GameObject.h"
#include "GridComponent.h"
#include "TextureComponent.h"
#include "GameTime.h"
#include <vector>
#include <cstdlib>
#include <glm/glm.hpp>

namespace dae
{
    PookaGhostState::PookaGhostState(GameObject* pGridObject, float ghostSpeed, std::string ghostTexture, std::string walkTexture)
        : m_pGridObject(pGridObject)
        , m_GhostSpeed(ghostSpeed)
        , m_GhostTexture(std::move(ghostTexture))
        , m_WalkTexture(std::move(walkTexture))
    {}

    void PookaGhostState::OnEnter(GameObject* owner)
    {
        if (auto* tex = owner->GetComponent<TextureComponent>())
            tex->SetTexture(m_GhostTexture);

        m_HasTarget = false;

        if (!m_pGridObject) return;
        auto* grid = m_pGridObject->GetComponent<GridComponent>();
        if (!grid) return;

        const glm::vec3 gridOrigin = m_pGridObject->GetWorldPosition();
        const glm::vec3 worldPos   = owner->GetWorldPosition();

        // Snap to current cell centre – clean start position, no drift
        int currentCol{}, currentRow{};
        if (!grid->WorldToCell(worldPos.x - gridOrigin.x,
                               worldPos.y - gridOrigin.y,
                               currentCol, currentRow)) return;

        float cx{}, cy{};
        grid->CellToWorld(currentCol, currentRow, cx, cy);
        const glm::vec3 snappedPos = gridOrigin + glm::vec3(cx, cy, 0.f);
        owner->SetLocalPosition(snappedPos);

        // Collect all entered cells with at least one open side so the Pooka
        // can actually start walking again once it arrives there
        std::vector<std::pair<int, int>> candidates;
        for (int r = 0; r < grid->GetRows(); ++r)
        {
            for (int c = 0; c < grid->GetCols(); ++c)
            {
                if (c == currentCol && r == currentRow) continue;
                if (!grid->IsCellEntered(c, r)) continue;
                if (grid->IsSideOpen(c, r, TunnelSide::Right) ||
                    grid->IsSideOpen(c, r, TunnelSide::Left)  ||
                    grid->IsSideOpen(c, r, TunnelSide::Down)  ||
                    grid->IsSideOpen(c, r, TunnelSide::Up))
                    candidates.emplace_back(c, r);
            }
        }

        if (candidates.empty()) return; // nowhere walkable to go, stay in ghost

        const auto [tc, tr] = candidates[static_cast<size_t>(std::rand()) % candidates.size()];
        m_TargetCol = tc;
        m_TargetRow = tr;

        float tx{}, ty{};
        grid->CellToWorld(m_TargetCol, m_TargetRow, tx, ty);
        m_TargetWorldPos = gridOrigin + glm::vec3(tx, ty, 0.f);

        // Direction computed from the snapped (not drifted) position
        const glm::vec3 diff = m_TargetWorldPos - snappedPos;
        const float     len  = glm::length(diff);
        m_MoveDir   = len > 0.f ? diff / len : glm::vec3{ 1.f, 0.f, 0.f };
        m_HasTarget = true;
    }

    std::unique_ptr<PookaState> PookaGhostState::Update(GameObject* owner)
    {
        if (!m_HasTarget) return nullptr;

        const float     step       = m_GhostSpeed * GameTime::GetInstance().GetDeltaTime();
        const glm::vec3 currentPos = owner->GetWorldPosition();
        const glm::vec3 toTarget   = m_TargetWorldPos - currentPos;
        const float     dist       = glm::length(toTarget);

        if (step >= dist)
        {
            // Snap exactly to target cell centre, then hand off to walking state
            owner->SetLocalPosition(m_TargetWorldPos);
            return std::make_unique<PookaWalkingState>(m_pGridObject, 10.f, 60.f, m_WalkTexture, m_GhostTexture);
        }

        owner->SetLocalPosition(currentPos + m_MoveDir * step);
        return nullptr;
    }
}