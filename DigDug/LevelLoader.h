#pragma once
#include "LevelData.h"
#include <filesystem>
#include <optional>

namespace dae
{
    class LevelLoader
    {
    public:
        static std::optional<LevelData> Load(const std::filesystem::path& filePath);

        static LevelData MakeDefault();
    };
}