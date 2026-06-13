#pragma once
#include "BaseComponent.h"
#include <vector>
#include <glm/glm.hpp>

namespace dae
{
    static constexpr int kFixedCols = 14;
    static constexpr int kFixedRows = 16;
    static constexpr int kGroundRow =  0;

    enum class TunnelSide : uint8_t
    {
        None  = 0,
        Up    = 1 << 0,
        Down  = 1 << 1,
        Left  = 1 << 2,
        Right = 1 << 3,
    };
    inline TunnelSide  operator| (TunnelSide a, TunnelSide b)
    { return static_cast<TunnelSide>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b)); }
    inline TunnelSide& operator|=(TunnelSide& a, TunnelSide b) { return a = a | b; }
    inline bool HasSide(TunnelSide mask, TunnelSide flag)
    { return (static_cast<uint8_t>(mask) & static_cast<uint8_t>(flag)) != 0; }

    struct GridCell
    {
        TunnelSide openSides  { TunnelSide::None };
        uint16_t   dugSubCells{ 0 };
    };

    class GridComponent final : public BaseComponent
    {
    public:
        GridComponent(GameObject* owner,
                      float totalWidth, float totalHeight,
                      int cols, int rows);
        ~GridComponent() override = default;

        void FixedUpdate(float) override {}
        void Update()           override {}
        void LateUpdate()       override {}
        void Render() const     override;

        int   GetCols()        const { return m_Cols; }
        int   GetRows()        const { return m_Rows; }
        float GetCellWidth()   const { return m_CellWidth; }
        float GetCellHeight()  const { return m_CellHeight; }
        float GetTotalWidth()  const { return m_TotalWidth; }
        float GetTotalHeight() const { return m_TotalHeight; }

        float GetSubCellWidth()  const { return m_CellWidth  / 3.f; }
        float GetSubCellHeight() const { return m_CellHeight / 3.f; }
        int   GetSubCols()       const { return m_Cols * 3; }
        int   GetSubRows()       const { return m_Rows * 3; }

        bool IsGroundRow(int row) const { return row == kGroundRow; }
        bool IsGroundSubRow(int subRow) const
        {
            return subRow >= kGroundRow * 3 && subRow < (kGroundRow + 1) * 3;
        }

        bool WorldToCell(float wx, float wy, int& outCol, int& outRow) const;
        void CellToWorld(int col, int row, float& outX, float& outY)   const;
        bool IsValid(int col, int row) const;

        bool WorldToSubCell(float wx, float wy, int& outSubCol, int& outSubRow) const;
        void SubCellToWorld(int subCol, int subRow, float& outX, float& outY)   const;
        bool IsSubCellValid(int subCol, int subRow) const;

        TunnelSide GetOpenSides(int col, int row) const;
        void       OpenSide(int col, int row, TunnelSide side);
        bool       IsSideOpen(int col, int row, TunnelSide side) const;

        void DigSubCell(int subCol, int subRow);
        void DigPlayerStep(int targetSubCol, int targetSubRow, int directionX, int directionY);
        void DigPlayerArea(int centerSubCol, int centerSubRow);

        bool IsSubCellDug (int subCol, int subRow) const;
        bool IsCellEntered(int col, int row) const;
        bool IsCellDug    (int col, int row) const;

        void DigSubCellsFullyInside(float relLeft, float relTop, float relRight, float relBottom);
        bool IsBoxTouchingUndugSubCell(float relLeft, float relTop, float relRight, float relBottom) const;

        void PreDigCell   (int col, int row, TunnelSide sides);
        void PreDigSubCell(int subCol, int subRow) { DigSubCell(subCol, subRow); }

    private:
        int   m_Cols{};
        int   m_Rows{};
        float m_TotalWidth{};
        float m_TotalHeight{};
        float m_CellWidth{};
        float m_CellHeight{};

        std::vector<GridCell> m_Cells;

        GridCell*       Cell(int col, int row);
        const GridCell* Cell(int col, int row) const;
    };
}