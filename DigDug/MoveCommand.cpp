#include "MoveCommand.h"
#include "GameObject.h"
#include "GridComponent.h"
#include <cmath>

namespace dae
{
    MoveCommand::MoveCommand(GameObject* pGameObject, glm::vec2 direction,
                             GameObject* pGridObject)
        : GameObjectCommand(pGameObject)
        , m_Direction(direction)
        , m_pGridObject(pGridObject)
    {}

    void MoveCommand::Execute()
    {
        auto* actor = GetGameObject();
        if (!actor || !m_pGridObject) return;

        auto* grid = m_pGridObject->GetComponent<GridComponent>();
        if (!grid) return;

        const glm::vec3 gridOrigin = m_pGridObject->GetWorldPosition();
        const glm::vec3 worldPos   = actor->GetWorldPosition();
        const float relativeX = worldPos.x - gridOrigin.x;
        const float relativeY = worldPos.y - gridOrigin.y;

        int subCol{}, subRow{};
        if (!grid->WorldToSubCell(relativeX, relativeY, subCol, subRow)) return;

        const int subColMod = subCol % 3;
        const int subRowMod = subRow % 3;

        if (subColMod != 1 && subRowMod != 1) return;
        if (subColMod != 1 && m_Direction.x == 0.f) return;
        if (subRowMod != 1 && m_Direction.y == 0.f) return;

        const int directionX = static_cast<int>(m_Direction.x);
        const int directionY = static_cast<int>(m_Direction.y);

        const int targetSubCol = subCol + directionX;
        const int targetSubRow = subRow + directionY;

        const int subColMax = grid->GetSubCols() - 2;
        const int subRowMax = grid->GetSubRows() - 2;
        if (targetSubCol < 1 || targetSubCol > subColMax) return;
        if (targetSubRow < 1 || targetSubRow > subRowMax) return;

        // ── Snap ─────────────────────────────────────────────────────────────
        float newRelativeX{}, newRelativeY{};
        grid->SubCellToWorld(targetSubCol, targetSubRow, newRelativeX, newRelativeY);
        actor->SetLocalPosition(gridOrigin + glm::vec3(newRelativeX, newRelativeY, 0.f));

        // ── Dig the 3x3 block around the new centre ───────────────────────────
        grid->DigPlayerArea(targetSubCol, targetSubRow);

        // ── Open sides for every big-cell boundary the 3x3 body straddles ────
        // The leading edge of the player is 1 subcell ahead of the centre.
        // So check the centre AND the leading edge subcell for boundary crossings.
        const int leadingSubCol = targetSubCol + directionX; // one step further in move dir
        const int leadingSubRow = targetSubRow + directionY;

        const int centerBigCol = targetSubCol / 3;
        const int centerBigRow = targetSubRow / 3;
        const int leadingBigCol = (leadingSubCol >= 0 && leadingSubCol < grid->GetSubCols()) ? leadingSubCol / 3 : centerBigCol;
        const int leadingBigRow = (leadingSubRow >= 0 && leadingSubRow < grid->GetSubRows()) ? leadingSubRow / 3 : centerBigRow;

        // Open for the centre crossing (same as before)
        const int previousBigCol = subCol / 3;
        const int previousBigRow = subRow / 3;
        if (previousBigCol != centerBigCol || previousBigRow != centerBigRow)
        {
            TunnelSide exitSide  = TunnelSide::None;
            TunnelSide entrySide = TunnelSide::None;
            if      (directionX > 0) { exitSide = TunnelSide::Right; entrySide = TunnelSide::Left;  }
            else if (directionX < 0) { exitSide = TunnelSide::Left;  entrySide = TunnelSide::Right; }
            else if (directionY > 0) { exitSide = TunnelSide::Down;  entrySide = TunnelSide::Up;    }
            else if (directionY < 0) { exitSide = TunnelSide::Up;    entrySide = TunnelSide::Down;  }
            grid->OpenSide(previousBigCol, previousBigRow, exitSide);
            grid->OpenSide(centerBigCol,   centerBigRow,   entrySide);
        }

        // Open for the leading edge crossing (1 subcell ahead of centre)
        if (centerBigCol != leadingBigCol || centerBigRow != leadingBigRow)
        {
            TunnelSide exitSide  = TunnelSide::None;
            TunnelSide entrySide = TunnelSide::None;
            if      (directionX > 0) { exitSide = TunnelSide::Right; entrySide = TunnelSide::Left;  }
            else if (directionX < 0) { exitSide = TunnelSide::Left;  entrySide = TunnelSide::Right; }
            else if (directionY > 0) { exitSide = TunnelSide::Down;  entrySide = TunnelSide::Up;    }
            else if (directionY < 0) { exitSide = TunnelSide::Up;    entrySide = TunnelSide::Down;  }
            grid->OpenSide(centerBigCol,  centerBigRow,  exitSide);
            grid->OpenSide(leadingBigCol, leadingBigRow, entrySide);
        }
    }
}