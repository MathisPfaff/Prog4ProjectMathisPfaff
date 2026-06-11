#include "GridComponent.h"
#include "GameObject.h"
#include "Renderer.h"
#include <SDL3/SDL.h>
#include <cassert>
#include <cmath>
#include <algorithm>

namespace dae
{
    // ── Tunnel colour (black) ─────────────────────────────────────────────────
    static constexpr SDL_Color kColorTunnel{   0,   0,   0, 255 };

    // ── Ground (sky) row colour – always solid blue ───────────────────────────
    static constexpr SDL_Color kColorGround{  40, 40, 200, 255 };

    // ── Underground zone dirt colours ─────────────────────────────────────────
    // Level 1 – rows 1-4   : Yellow
    static constexpr SDL_Color kColorZone1 { 210, 170,  30, 255 };
    // Level 2 – rows 5-8   : Orange
    static constexpr SDL_Color kColorZone2 { 210, 105,  30, 255 };
    // Level 3 – rows 9-12  : Dark Orange
    static constexpr SDL_Color kColorZone3 { 170,  65,  15, 255 };
    // Level 4 – rows 13-15 : Dark Red
    static constexpr SDL_Color kColorZone4 { 130,  20,  10, 255 };

    // Thickness (px) of tunnel-border walls drawn as filled rects
    static constexpr float kWallThickness = 2.f;

    // Returns the zone colour for a given big-cell row.
    // Row 0 (ground/sky) → blue; deeper rows get progressively darker.
    static SDL_Color DirtColorForRow(int row)
    {
        if (row <= 0)  return kColorGround;
        if (row <= 4)  return kColorZone1;
        if (row <= 8)  return kColorZone2;
        if (row <= 12) return kColorZone3;
        return kColorZone4;
    }

    // Wall colour: use the deeper (higher row-index) of the two touching zones.
    static SDL_Color WallColor(int bigRowA, int bigRowB)
    {
        return DirtColorForRow(std::max(bigRowA, bigRowB));
    }

    // ─────────────────────────────────────────────────────────────────────────

    GridComponent::GridComponent(GameObject* owner,
                                 float totalWidth, float totalHeight,
                                 int cols, int rows)
        : BaseComponent(owner)
        , m_Cols(cols)
        , m_Rows(rows)
        , m_TotalWidth(totalWidth)
        , m_TotalHeight(totalHeight)
        , m_CellWidth (totalWidth  / static_cast<float>(cols))
        , m_CellHeight(totalHeight / static_cast<float>(rows))
        , m_Cells(static_cast<size_t>(cols * rows))
    {
        assert(cols == kFixedCols && rows == kFixedRows &&
               "GridComponent: grid must be exactly kFixedCols x kFixedRows (14x16)");
        assert(cols > 0 && rows > 0 && "GridComponent: cols and rows must be > 0");

        // Ground row (row 0) is always fully open – pre-open all its sub-cells
        for (int col = 0; col < m_Cols; ++col)
        {
            const int baseSubCol = col * 3;
            const int baseSubRow = kGroundRow * 3;
            for (int localSubRow = 0; localSubRow < 3; ++localSubRow)
                for (int localSubCol = 0; localSubCol < 3; ++localSubCol)
                    DigSubCell(baseSubCol + localSubCol, baseSubRow + localSubRow);

            OpenSide(col, kGroundRow, TunnelSide::Left | TunnelSide::Right);
        }
    }

    GridCell* GridComponent::Cell(int col, int row)
    {
        if (!IsValid(col, row)) return nullptr;
        return &m_Cells[static_cast<size_t>(row * m_Cols + col)];
    }

    const GridCell* GridComponent::Cell(int col, int row) const
    {
        if (!IsValid(col, row)) return nullptr;
        return &m_Cells[static_cast<size_t>(row * m_Cols + col)];
    }

    bool GridComponent::IsValid(int col, int row) const
    {
        return col >= 0 && col < m_Cols && row >= 0 && row < m_Rows;
    }

    bool GridComponent::WorldToCell(float wx, float wy, int& outCol, int& outRow) const
    {
        outCol = static_cast<int>(std::floor(wx / m_CellWidth));
        outRow = static_cast<int>(std::floor(wy / m_CellHeight));
        return IsValid(outCol, outRow);
    }

    void GridComponent::CellToWorld(int col, int row, float& outX, float& outY) const
    {
        outX = (static_cast<float>(col) * 3.f + 1.5f) * GetSubCellWidth();
        outY = (static_cast<float>(row) * 3.f + 1.5f) * GetSubCellHeight();
    }

    bool GridComponent::IsSubCellValid(int subCol, int subRow) const
    {
        return subCol >= 0 && subCol < GetSubCols() && subRow >= 0 && subRow < GetSubRows();
    }

    bool GridComponent::WorldToSubCell(float wx, float wy, int& outSubCol, int& outSubRow) const
    {
        outSubCol = static_cast<int>(std::floor(wx / GetSubCellWidth()));
        outSubRow = static_cast<int>(std::floor(wy / GetSubCellHeight()));
        return IsSubCellValid(outSubCol, outSubRow);
    }

    void GridComponent::SubCellToWorld(int subCol, int subRow, float& outX, float& outY) const
    {
        outX = (static_cast<float>(subCol) + 0.5f) * GetSubCellWidth();
        outY = (static_cast<float>(subRow) + 0.5f) * GetSubCellHeight();
    }

    TunnelSide GridComponent::GetOpenSides(int col, int row) const
    {
        const auto* cell = Cell(col, row);
        return cell ? cell->openSides : TunnelSide::None;
    }

    void GridComponent::OpenSide(int col, int row, TunnelSide side)
    {
        if (auto* cell = Cell(col, row)) cell->openSides |= side;
    }

    bool GridComponent::IsSideOpen(int col, int row, TunnelSide side) const
    {
        return HasSide(GetOpenSides(col, row), side);
    }

    void GridComponent::DigSubCell(int subCol, int subRow)
    {
        if (!IsSubCellValid(subCol, subRow)) return;

        const int col = subCol / 3;
        const int row = subRow / 3;
        const int localCol = subCol % 3;
        const int localRow = subRow % 3;
        const int bitIndex = localRow * 3 + localCol;

        auto* cell = Cell(col, row);
        if (!cell) return;
        if (cell->dugSubCells & static_cast<uint16_t>(1 << bitIndex)) return;

        cell->dugSubCells |= static_cast<uint16_t>(1 << bitIndex);
    }

    void GridComponent::DigPlayerStep(int targetSubCol, int targetSubRow, int directionX, int directionY)
    {
        // Never dig the ground row
        if (IsGroundSubRow(targetSubRow)) return;

        if (directionX != 0)
        {
            DigSubCell(targetSubCol, targetSubRow - 1);
            DigSubCell(targetSubCol, targetSubRow    );
            DigSubCell(targetSubCol, targetSubRow + 1);

            DigSubCell(targetSubCol - directionX, targetSubRow - 1);
            DigSubCell(targetSubCol - directionX, targetSubRow    );
            DigSubCell(targetSubCol - directionX, targetSubRow + 1);
        }
        else if (directionY != 0)
        {
            DigSubCell(targetSubCol - 1, targetSubRow);
            DigSubCell(targetSubCol,     targetSubRow);
            DigSubCell(targetSubCol + 1, targetSubRow);

            DigSubCell(targetSubCol - 1, targetSubRow - directionY);
            DigSubCell(targetSubCol,     targetSubRow - directionY);
            DigSubCell(targetSubCol + 1, targetSubRow - directionY);
        }
    }

    void GridComponent::DigPlayerArea(int centerSubCol, int centerSubRow)
    {
        // Never dig the ground row
        if (IsGroundSubRow(centerSubRow)) return;

        for (int deltaSubRow = -1; deltaSubRow <= 1; ++deltaSubRow)
            for (int deltaSubCol = -1; deltaSubCol <= 1; ++deltaSubCol)
                DigSubCell(centerSubCol + deltaSubCol, centerSubRow + deltaSubRow);
    }

    bool GridComponent::IsSubCellDug(int subCol, int subRow) const
    {
        if (!IsSubCellValid(subCol, subRow)) return false;
        const int col      = subCol / 3;
        const int row      = subRow / 3;
        const int localCol = subCol % 3;
        const int localRow = subRow % 3;
        const int bitIndex = localRow * 3 + localCol;
        const auto* cell = Cell(col, row);
        return cell && (cell->dugSubCells & static_cast<uint16_t>(1 << bitIndex)) != 0;
    }

    bool GridComponent::IsCellEntered(int col, int row) const
    {
        const auto* cell = Cell(col, row);
        return cell && cell->dugSubCells != 0;
    }

    bool GridComponent::IsCellDug(int col, int row) const
    {
        const auto* cell = Cell(col, row);
        return cell && (cell->dugSubCells & 0x1FF) == 0x1FF;
    }

    void GridComponent::DigSubCellsFullyInside(float relLeft, float relTop, float relRight, float relBottom)
    {
        const float sw = GetSubCellWidth();
        const float sh = GetSubCellHeight();

        // A subcell sc is FULLY inside when: sc*sw >= relLeft  AND  (sc+1)*sw <= relRight
        const int minSubCol = static_cast<int>(std::ceil(relLeft / sw));
        const int maxSubCol = static_cast<int>(std::floor(relRight / sw)) - 1;
        const int minSubRow = static_cast<int>(std::ceil(relTop / sh));
        const int maxSubRow = static_cast<int>(std::floor(relBottom / sh)) - 1;

        for (int sr = minSubRow; sr <= maxSubRow; ++sr)
        {
            if (IsGroundSubRow(sr)) continue;
            for (int sc = minSubCol; sc <= maxSubCol; ++sc)
                DigSubCell(sc, sr);
        }
    }

    bool GridComponent::IsBoxTouchingUndugSubCell(float relLeft, float relTop, float relRight, float relBottom) const
    {
        const float sw = GetSubCellWidth();
        const float sh = GetSubCellHeight();

        // A subcell sc OVERLAPS the rect when: sc*sw < relRight  AND  (sc+1)*sw > relLeft
        const int minSubCol = static_cast<int>(std::floor(relLeft / sw));
        const int maxSubCol = static_cast<int>(std::ceil(relRight / sw)) - 1;
        const int minSubRow = static_cast<int>(std::floor(relTop / sh));
        const int maxSubRow = static_cast<int>(std::ceil(relBottom / sh)) - 1;

        for (int sr = minSubRow; sr <= maxSubRow; ++sr)
        {
            if (IsGroundSubRow(sr)) continue;
            for (int sc = minSubCol; sc <= maxSubCol; ++sc)
                if (!IsSubCellDug(sc, sr)) return true;
        }
        return false;
    }

    void GridComponent::PreDigCell(int col, int row, TunnelSide sides)
    {
        if (!IsValid(col, row)) return;
        // Protect the ground row from accidental pre-digging calls
        if (IsGroundRow(row)) return;

        const int baseSubCol = col * 3;
        const int baseSubRow = row * 3;

        for (int localSubRow = 0; localSubRow < 3; ++localSubRow)
            for (int localSubCol = 0; localSubCol < 3; ++localSubCol)
                DigSubCell(baseSubCol + localSubCol, baseSubRow + localSubRow);

        if (HasSide(sides, TunnelSide::Left))
        {
            for (int localSubRow = 0; localSubRow < 3; ++localSubRow)
                DigSubCell(baseSubCol - 1, baseSubRow + localSubRow);
            OpenSide(col,     row, TunnelSide::Left);
            OpenSide(col - 1, row, TunnelSide::Right);
        }
        if (HasSide(sides, TunnelSide::Right))
        {
            for (int localSubRow = 0; localSubRow < 3; ++localSubRow)
                DigSubCell(baseSubCol + 3, baseSubRow + localSubRow);
            OpenSide(col,     row, TunnelSide::Right);
            OpenSide(col + 1, row, TunnelSide::Left);
        }
        if (HasSide(sides, TunnelSide::Up))
        {
            for (int localSubCol = 0; localSubCol < 3; ++localSubCol)
                DigSubCell(baseSubCol + localSubCol, baseSubRow - 1);
            OpenSide(col, row,     TunnelSide::Up);
            OpenSide(col, row - 1, TunnelSide::Down);
        }
        if (HasSide(sides, TunnelSide::Down))
        {
            for (int localSubCol = 0; localSubCol < 3; ++localSubCol)
                DigSubCell(baseSubCol + localSubCol, baseSubRow + 3);
            OpenSide(col, row,     TunnelSide::Down);
            OpenSide(col, row + 1, TunnelSide::Up);
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    //  Render
    // ─────────────────────────────────────────────────────────────────────────
    void GridComponent::Render() const
    {
        auto* renderer = dae::Renderer::GetInstance().GetSDLRenderer();
        const glm::vec3 worldPosition = GetOwner()->GetWorldPosition();
        const float originX = worldPosition.x;
        const float originY = worldPosition.y;

        const float subCellWidth  = GetSubCellWidth();
        const float subCellHeight = GetSubCellHeight();
        const int   totalSubCols  = GetSubCols();
        const int   totalSubRows  = GetSubRows();

        // ── 1. Background zone bands (all rows; row 0 draws as solid blue) ────
        for (int row = 0; row < m_Rows; ++row)
        {
            const SDL_Color c = DirtColorForRow(row);
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            const SDL_FRect band{
                originX,
                originY + static_cast<float>(row) * m_CellHeight,
                m_TotalWidth,
                m_CellHeight
            };
            SDL_RenderFillRect(renderer, &band);
        }

        // ── 2. Dug tunnel sub-cells – always black ────────────────────────────
        SDL_SetRenderDrawColor(renderer,
            kColorTunnel.r, kColorTunnel.g, kColorTunnel.b, kColorTunnel.a);

        for (int subRow = 0; subRow < totalSubRows; ++subRow)
        {
            if (IsGroundSubRow(subRow)) continue;  // ground row stays its blue colour

            for (int subCol = 0; subCol < totalSubCols; ++subCol)
            {
                if (!IsSubCellDug(subCol, subRow)) continue;

                const SDL_FRect rect{
                    originX + static_cast<float>(subCol) * subCellWidth,
                    originY + static_cast<float>(subRow) * subCellHeight,
                    subCellWidth, subCellHeight
                };
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        // ── 3. Tunnel border walls – thick filled rects in zone colour ────────
        //  Colour rule: use the deeper (higher row index) of the two touching zones.
        //  Thickness: kWallThickness pixels drawn INTO the dug (black) cell.
        for (int subRow = 0; subRow < totalSubRows; ++subRow)
        {
            if (IsGroundSubRow(subRow)) continue;

            for (int subCol = 0; subCol < totalSubCols; ++subCol)
            {
                if (!IsSubCellDug(subCol, subRow)) continue;

                const float left   = originX + static_cast<float>(subCol)     * subCellWidth;
                const float top    = originY + static_cast<float>(subRow)     * subCellHeight;
                const float right  = originX + static_cast<float>(subCol + 1) * subCellWidth;
                const float bottom = originY + static_cast<float>(subRow + 1) * subCellHeight;
                const float sw     = subCellWidth;
                const float sh     = subCellHeight;

                const int bigCol = subCol / 3;
                const int bigRow = subRow / 3;

                // ── Top edge ─────────────────────────────────────────────────
                {
                    const int neighborSubRow = subRow - 1;
                    const int neighborBigRow = (neighborSubRow >= 0) ? neighborSubRow / 3 : bigRow;

                    bool draw = false;
                    if (!IsSubCellDug(subCol, neighborSubRow))
                        draw = true;
                    else if (subRow % 3 == 0 && !IsSideOpen(bigCol, bigRow, TunnelSide::Up))
                        draw = true;

                    if (draw)
                    {
                        const SDL_Color wc = WallColor(bigRow, neighborBigRow);
                        SDL_SetRenderDrawColor(renderer, wc.r, wc.g, wc.b, wc.a);
                        const SDL_FRect wall{ left, top, sw, kWallThickness };
                        SDL_RenderFillRect(renderer, &wall);
                    }
                }

                // ── Bottom edge ───────────────────────────────────────────────
                {
                    const int neighborSubRow = subRow + 1;
                    const int neighborBigRow = (neighborSubRow < totalSubRows) ? neighborSubRow / 3 : bigRow;

                    bool draw = false;
                    if (!IsSubCellDug(subCol, neighborSubRow))
                        draw = true;
                    else if (subRow % 3 == 2 && !IsSideOpen(bigCol, bigRow, TunnelSide::Down))
                        draw = true;

                    if (draw)
                    {
                        const SDL_Color wc = WallColor(bigRow, neighborBigRow);
                        SDL_SetRenderDrawColor(renderer, wc.r, wc.g, wc.b, wc.a);
                        const SDL_FRect wall{ left, bottom - kWallThickness, sw, kWallThickness };
                        SDL_RenderFillRect(renderer, &wall);
                    }
                }

                // ── Left edge ────────────────────────────────────────────────
                {
                    const int neighborSubCol = subCol - 1;

                    bool draw = false;
                    if (!IsSubCellDug(neighborSubCol, subRow))
                        draw = true;
                    else if (subCol % 3 == 0 && !IsSideOpen(bigCol, bigRow, TunnelSide::Left))
                        draw = true;

                    if (draw)
                    {
                        const SDL_Color wc = DirtColorForRow(bigRow);
                        SDL_SetRenderDrawColor(renderer, wc.r, wc.g, wc.b, wc.a);
                        const SDL_FRect wall{ left, top, kWallThickness, sh };
                        SDL_RenderFillRect(renderer, &wall);
                    }
                }

                // ── Right edge ────────────────────────────────────────────────
                {
                    const int neighborSubCol = subCol + 1;

                    bool draw = false;
                    if (!IsSubCellDug(neighborSubCol, subRow))
                        draw = true;
                    else if (subCol % 3 == 2 && !IsSideOpen(bigCol, bigRow, TunnelSide::Right))
                        draw = true;

                    if (draw)
                    {
                        const SDL_Color wc = DirtColorForRow(bigRow);
                        SDL_SetRenderDrawColor(renderer, wc.r, wc.g, wc.b, wc.a);
                        const SDL_FRect wall{ right - kWallThickness, top, kWallThickness, sh };
                        SDL_RenderFillRect(renderer, &wall);
                    }
                }
            }
        }
    }
}