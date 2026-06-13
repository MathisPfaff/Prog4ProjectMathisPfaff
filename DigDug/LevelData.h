#pragma once
#include <vector>
#include <cstdint>

namespace dae
{
    // ── Enemy type tag stored in the binary file ──────────────────────────────
    enum class EnemyType : uint8_t
    {
        Pooka = 0,
        Fygar = 1,
    };

    // ── One enemy entry in the file ───────────────────────────────────────────
    struct EnemyEntry
    {
        EnemyType type{};
        uint8_t   col{};
        uint8_t   row{};
    };

    // ── Everything the PlayingState needs to build the scene ──────────────────
    struct LevelData
    {
        uint8_t player1Col{ 1 };
        uint8_t player1Row{ 1 };
        uint8_t player2Col{ 12 };
        uint8_t player2Row{ 1 };

        std::vector<EnemyEntry> enemies;
    };
}