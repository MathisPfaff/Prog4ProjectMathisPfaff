#include "LevelLoader.h"
#include <fstream>
#include <cstring>

namespace dae
{
    // ── Internal format constants ─────────────────────────────────────────────
    static constexpr char    kMagic[4] = { 'D', 'D', 'L', 'V' };
    static constexpr uint8_t kVersion = 1;

    std::optional<LevelData> LevelLoader::Load(const std::filesystem::path& filePath)
    {
        std::ifstream f(filePath, std::ios::binary);
        if (!f.is_open()) return std::nullopt;

        // Magic
        char magic[4]{};
        f.read(magic, 4);
        if (!f || std::memcmp(magic, kMagic, 4) != 0) return std::nullopt;

        // Version
        uint8_t version{};
        f.read(reinterpret_cast<char*>(&version), 1);
        if (!f || version != kVersion) return std::nullopt;

        LevelData data{};

        // Player spawn positions
        f.read(reinterpret_cast<char*>(&data.player1Col), 1);
        f.read(reinterpret_cast<char*>(&data.player1Row), 1);
        f.read(reinterpret_cast<char*>(&data.player2Col), 1);
        f.read(reinterpret_cast<char*>(&data.player2Row), 1);
        if (!f) return std::nullopt;

        // Enemy list
        uint8_t enemyCount{};
        f.read(reinterpret_cast<char*>(&enemyCount), 1);
        if (!f) return std::nullopt;

        data.enemies.reserve(enemyCount);
        for (uint8_t i = 0; i < enemyCount; ++i)
        {
            uint8_t typeVal{}, col{}, row{};
            f.read(reinterpret_cast<char*>(&typeVal), 1);
            f.read(reinterpret_cast<char*>(&col), 1);
            f.read(reinterpret_cast<char*>(&row), 1);
            if (!f) return std::nullopt;

            data.enemies.push_back({ static_cast<EnemyType>(typeVal), col, row });
        }

        return data;
    }

    LevelData LevelLoader::MakeDefault()
    {
        LevelData data{};
        data.player1Col = 1;  data.player1Row = 1;
        data.player2Col = 12; data.player2Row = 1;
        data.enemies =
        {
            { EnemyType::Pooka,  3, 1 },
            { EnemyType::Pooka,  9, 9 },
            { EnemyType::Fygar,  9, 5 },
        };
        return data;
    }
}