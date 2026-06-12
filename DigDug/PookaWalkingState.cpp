#include "PookaWalkingState.h"
#include "PookaGhostState.h"
#include "GameObject.h"
#include "GridComponent.h"
#include "TextureComponent.h"
#include "GameTime.h"
#include <array>
#include <vector>
#include <cstdlib>
#include <glm/glm.hpp>

namespace
{
    struct DirEntry { int dx, dy; dae::TunnelSide side; };
    constexpr std::array<DirEntry, 4> kDirs{{
        {  1,  0, dae::TunnelSide::Right },
        { -1,  0, dae::TunnelSide::Left  },
        {  0,  1, dae::TunnelSide::Down  },
        {  0, -1, dae::TunnelSide::Up    },
    }};

    bool CanEnterCell(dae::GridComponent* grid, int fromCol, int fromRow,
                      int toCol, int toRow, dae::TunnelSide side)
    {
        if (!grid->IsValid(toCol, toRow)) return false;
        if (!grid->IsSideOpen(fromCol, fromRow, side)) return false;
        const int centerSubCol = toCol * 3 + 1;
        const int centerSubRow = toRow * 3 + 1;
        return grid->IsSubCellDug(centerSubCol, centerSubRow);
    }

    // Returns true when the enemy can immediately start moving from (col, row).
    bool IsWalkable(dae::GridComponent* grid, int col, int row)
    {
        for (const auto& d : kDirs)
            if (CanEnterCell(grid, col, row, col + d.dx, row + d.dy, d.side))
                return true;
        return false;
    }

    // BFS from (startCol, startRow) to find the nearest cell the enemy can
    // actually walk out of. Called when an enemy enters walking state from a
    // position that is not on a tunnel (e.g. was a ghost mid-wall when pumped).
    std::pair<int,int> FindNearestWalkableCell(dae::GridComponent* grid,
                                               int startCol, int startRow)
    {
        if (IsWalkable(grid, startCol, startRow))
            return { startCol, startRow };

        const int cols = grid->GetCols();
        const int rows = grid->GetRows();
        std::vector<bool> visited(static_cast<size_t>(rows * cols), false);
        std::vector<std::pair<int,int>> frontier{ { startCol, startRow } };
        visited[static_cast<size_t>(startRow * cols + startCol)] = true;

        while (!frontier.empty())
        {
            std::vector<std::pair<int,int>> next;
            for (auto [c, r] : frontier)
            {
                for (const auto& d : kDirs)
                {
                    const int nc = c + d.dx;
                    const int nr = r + d.dy;
                    if (!grid->IsValid(nc, nr)) continue;
                    const size_t idx = static_cast<size_t>(nr * cols + nc);
                    if (visited[idx]) continue;
                    visited[idx] = true;
                    if (IsWalkable(grid, nc, nr))
                        return { nc, nr };
                    next.emplace_back(nc, nr);
                }
            }
            frontier = std::move(next);
        }
        return { startCol, startRow }; // nothing found, fall back to original
    }
}

namespace dae
{
    PookaWalkingState::PookaWalkingState(GameObject* pGridObject,
                                         float walkDuration,
                                         float speed,
                                         std::string walkTexture,
                                         std::string ghostTexture)
        : m_pGridObject(pGridObject)
        , m_WalkDuration(walkDuration)
        , m_Speed(speed)
        , m_WalkTexture(std::move(walkTexture))
        , m_GhostTexture(std::move(ghostTexture))
    {}

    void PookaWalkingState::OnEnter(GameObject* owner)
    {
        if (auto* tex = owner->GetComponent<TextureComponent>())
            tex->SetTexture(m_WalkTexture);

        m_Timer     = 0.f;
        m_HasTarget = false;

        if (!m_pGridObject) return;
        auto* grid = m_pGridObject->GetComponent<GridComponent>();
        if (!grid) return;

        const glm::vec3 gridOrigin = m_pGridObject->GetWorldPosition();
        const glm::vec3 worldPos   = owner->GetWorldPosition();

        int col{}, row{};
        if (!grid->WorldToCell(worldPos.x - gridOrigin.x,
                               worldPos.y - gridOrigin.y, col, row)) return;

        // If the nearest cell has no walkable tunnel (e.g. the enemy was a ghost
        // mid-wall when the pump caught it), find the closest cell that does.
        std::tie(col, row) = FindNearestWalkableCell(grid, col, row);

        // Snap to exact cell centre – eliminates any drift from prior state
        float cx{}, cy{};
        grid->CellToWorld(col, row, cx, cy);
        owner->SetLocalPosition(gridOrigin + glm::vec3(cx, cy, 0.f));

        // Pick any fully-passable direction (no previous direction constraint on entry)
        std::vector<int> candidates;
        for (int i = 0; i < 4; ++i)
        {
            const int nc = col + kDirs[i].dx;
            const int nr = row + kDirs[i].dy;
            if (CanEnterCell(grid, col, row, nc, nr, kDirs[i].side))
                candidates.push_back(i);
        }
        if (candidates.empty()) return; // no passable directions – stay still

        const int chosen = candidates[static_cast<size_t>(std::rand()) % candidates.size()];
        m_DirX      = kDirs[chosen].dx;
        m_DirY      = kDirs[chosen].dy;
        m_TargetCol = col + m_DirX;
        m_TargetRow = row + m_DirY;

        float tx{}, ty{};
        grid->CellToWorld(m_TargetCol, m_TargetRow, tx, ty);
        m_TargetWorldPos = gridOrigin + glm::vec3(tx, ty, 0.f);
        m_HasTarget = true;
    }

    std::unique_ptr<PookaState> PookaWalkingState::Update(GameObject* owner)
    {
        const float dt = GameTime::GetInstance().GetDeltaTime();
        m_Timer += dt;

        if (!m_HasTarget)
        {
            if (m_Timer >= m_WalkDuration && m_pGridObject)
                return std::make_unique<PookaGhostState>(m_pGridObject, 35.f, m_GhostTexture, m_WalkTexture);
            return nullptr;
        }

        const glm::vec3 currentPos = owner->GetWorldPosition();
        const glm::vec3 toTarget   = m_TargetWorldPos - currentPos;
        const float     dist       = glm::length(toTarget);
        const float     step       = m_Speed * dt;

        if (step >= dist)
        {
            owner->SetLocalPosition(m_TargetWorldPos);

            if (m_Timer >= m_WalkDuration)
                return std::make_unique<PookaGhostState>(m_pGridObject, 35.f, m_GhostTexture, m_WalkTexture);

            PickNextCell(m_TargetCol, m_TargetRow, m_DirX, m_DirY);
        }
        else
        {
            const glm::vec3 moveDir = toTarget / dist;
            owner->SetLocalPosition(currentPos + moveDir * step);
        }

        return nullptr;
    }

    void PookaWalkingState::PickNextCell(int col, int row, int prevDirX, int prevDirY)
    {
        auto* grid = m_pGridObject->GetComponent<GridComponent>();
        if (!grid) return;

        const int oppX = -prevDirX;
        const int oppY = -prevDirY;

        std::vector<int> candidates;
        int reverseIdx = -1;

        for (int i = 0; i < 4; ++i)
        {
            const int nc = col + kDirs[i].dx;
            const int nr = row + kDirs[i].dy;
            if (!CanEnterCell(grid, col, row, nc, nr, kDirs[i].side)) continue;

            if (kDirs[i].dx == oppX && kDirs[i].dy == oppY)
                reverseIdx = i;
            else
                candidates.push_back(i);
        }

        int chosen = -1;
        if (!candidates.empty())
            chosen = candidates[static_cast<size_t>(std::rand()) % candidates.size()];
        else if (reverseIdx != -1)
            chosen = reverseIdx;

        if (chosen == -1) { m_HasTarget = false; return; }

        m_DirX      = kDirs[chosen].dx;
        m_DirY      = kDirs[chosen].dy;
        m_TargetCol = col + m_DirX;
        m_TargetRow = row + m_DirY;

        float tx{}, ty{};
        grid->CellToWorld(m_TargetCol, m_TargetRow, tx, ty);
        m_TargetWorldPos = m_pGridObject->GetWorldPosition() + glm::vec3(tx, ty, 0.f);
        m_HasTarget = true;
    }
}