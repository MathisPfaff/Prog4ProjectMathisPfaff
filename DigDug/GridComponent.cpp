#include "GridComponent.h"
#include "GameObject.h"
#include "Renderer.h"
#include <SDL3/SDL.h>
#include <cassert>
#include <cmath>

namespace dae
{
    static constexpr SDL_Color kColorDirt  {  45,  30,  90, 255 };
    static constexpr SDL_Color kColorTunnel{ 175, 115,  55, 255 };
    static constexpr SDL_Color kColorWall  {  20,  10,  40, 255 };

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
        assert(cols > 0 && rows > 0 && "GridComponent: cols and rows must be > 0");
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

        const int col      = subCol / 3;
        const int row      = subRow / 3;
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

    void GridComponent::PreDigCell(int col, int row, TunnelSide sides)
    {
        if (!IsValid(col, row)) return;

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

    void GridComponent::Render() const
    {
        auto* renderer      = dae::Renderer::GetInstance().GetSDLRenderer();
        const glm::vec3 worldPosition = GetOwner()->GetWorldPosition();
        const float originX = worldPosition.x;
        const float originY = worldPosition.y;

        const float subCellWidth  = GetSubCellWidth();
        const float subCellHeight = GetSubCellHeight();
        const int   totalSubCols  = GetSubCols();
        const int   totalSubRows  = GetSubRows();

        SDL_SetRenderDrawColor(renderer,
            kColorDirt.r, kColorDirt.g, kColorDirt.b, kColorDirt.a);
        const SDL_FRect bgRect{ originX, originY, m_TotalWidth, m_TotalHeight };
        SDL_RenderFillRect(renderer, &bgRect);

        SDL_SetRenderDrawColor(renderer,
            kColorTunnel.r, kColorTunnel.g, kColorTunnel.b, kColorTunnel.a);
        for (int subRow = 0; subRow < totalSubRows; ++subRow)
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

        for (int subRow = 0; subRow < totalSubRows; ++subRow)
        {
            for (int subCol = 0; subCol < totalSubCols; ++subCol)
            {
                if (!IsSubCellDug(subCol, subRow)) continue;

                const float left   = originX + static_cast<float>(subCol)     * subCellWidth;
                const float top    = originY + static_cast<float>(subRow)     * subCellHeight;
                const float right  = originX + static_cast<float>(subCol + 1) * subCellWidth;
                const float bottom = originY + static_cast<float>(subRow + 1) * subCellHeight;

                const int bigCol = subCol / 3;
                const int bigRow = subRow / 3;

                {
                    const int neighborSubRow = subRow - 1;
                    if (!IsSubCellDug(subCol, neighborSubRow))
                    {
                        SDL_SetRenderDrawColor(renderer,
                            kColorWall.r, kColorWall.g, kColorWall.b, kColorWall.a);
                        SDL_RenderLine(renderer, left, top, right, top);
                    }
                    else if (subRow % 3 == 0)
                    {
                        if (!IsSideOpen(bigCol, bigRow, TunnelSide::Up))
                        {
                            SDL_SetRenderDrawColor(renderer,
                                kColorDirt.r, kColorDirt.g, kColorDirt.b, kColorDirt.a);
                            SDL_RenderLine(renderer, left, top, right, top);
                        }
                    }
                }

                {
                    const int neighborSubRow = subRow + 1;
                    if (!IsSubCellDug(subCol, neighborSubRow))
                    {
                        SDL_SetRenderDrawColor(renderer,
                            kColorWall.r, kColorWall.g, kColorWall.b, kColorWall.a);
                        SDL_RenderLine(renderer, left, bottom, right, bottom);
                    }
                    else if (subRow % 3 == 2)
                    {
                        if (!IsSideOpen(bigCol, bigRow, TunnelSide::Down))
                        {
                            SDL_SetRenderDrawColor(renderer,
                                kColorDirt.r, kColorDirt.g, kColorDirt.b, kColorDirt.a);
                            SDL_RenderLine(renderer, left, bottom, right, bottom);
                        }
                    }
                }

                {
                    const int neighborSubCol = subCol - 1;
                    if (!IsSubCellDug(neighborSubCol, subRow))
                    {
                        SDL_SetRenderDrawColor(renderer,
                            kColorWall.r, kColorWall.g, kColorWall.b, kColorWall.a);
                        SDL_RenderLine(renderer, left, top, left, bottom);
                    }
                    else if (subCol % 3 == 0)
                    {
                        if (!IsSideOpen(bigCol, bigRow, TunnelSide::Left))
                        {
                            SDL_SetRenderDrawColor(renderer,
                                kColorDirt.r, kColorDirt.g, kColorDirt.b, kColorDirt.a);
                            SDL_RenderLine(renderer, left, top, left, bottom);
                        }
                    }
                }

                {
                    const int neighborSubCol = subCol + 1;
                    if (!IsSubCellDug(neighborSubCol, subRow))
                    {
                        SDL_SetRenderDrawColor(renderer,
                            kColorWall.r, kColorWall.g, kColorWall.b, kColorWall.a);
                        SDL_RenderLine(renderer, right, top, right, bottom);
                    }
                    else if (subCol % 3 == 2)
                    {
                        if (!IsSideOpen(bigCol, bigRow, TunnelSide::Right))
                        {
                            SDL_SetRenderDrawColor(renderer,
                                kColorDirt.r, kColorDirt.g, kColorDirt.b, kColorDirt.a);
                            SDL_RenderLine(renderer, right, top, right, bottom);
                        }
                    }
                }
            }
        }
    }
}