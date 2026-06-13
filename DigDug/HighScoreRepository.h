#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace dae
{
    struct HighScoreEntry
    {
        std::string name{};   // 3-letter initials
        int         score{};
    };

    // Simple flat-text store: one "NAME SCORE" per line, sorted descending.
    class HighScoreRepository final
    {
    public:
        HighScoreRepository() = delete;

        static void Save(const std::filesystem::path& dataPath,
            const std::string& name, int score);

        static std::vector<HighScoreEntry> Load(const std::filesystem::path& dataPath);

    private:
        static constexpr const char* k_FileName = "HighScores.txt";
        static constexpr int         k_MaxEntries = 10;

        static std::filesystem::path FilePath(const std::filesystem::path& dataPath);
    };
}