#pragma once
#include "LevelData.h"
#include <filesystem>
#include <optional>

namespace dae
{
    class LevelLoader
    {
    public:
        /// Reads a .bin level file.  Returns nullopt if the file is missing or corrupt.
        static std::optional<LevelData> Load(const std::filesystem::path& filePath);

        /// Returns a hard-coded default that mirrors the original hand-written scene.
        /// Useful as a fallback and as a seed for LevelWriter.
        static LevelData MakeDefault();
    };
}