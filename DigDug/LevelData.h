#pragma once
#include <vector>
#include <cstdint>

namespace dae
{
    enum class EnemyType : uint8_t
    {
        Pooka = 0,
        Fygar = 1,
    };

    struct EnemyEntry
    {
        EnemyType type{};
        uint8_t   col{};
        uint8_t   row{};
    };

    struct LevelData
    {
        uint8_t player1Col{ 1 };
        uint8_t player1Row{ 1 };
        uint8_t player2Col{ 12 };
        uint8_t player2Row{ 1 };

        std::vector<EnemyEntry> enemies;
    };
}