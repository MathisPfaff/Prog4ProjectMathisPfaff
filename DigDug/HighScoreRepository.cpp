#include "HighScoreRepository.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

namespace dae
{
    std::filesystem::path HighScoreRepository::FilePath(
        const std::filesystem::path& dataPath)
    {
        return std::filesystem::absolute(dataPath) / k_FileName;
    }

    std::vector<HighScoreEntry> HighScoreRepository::Load(
        const std::filesystem::path& dataPath)
    {
        std::vector<HighScoreEntry> entries;

        std::ifstream file(FilePath(dataPath));
        if (!file.is_open())
            return entries;

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty()) continue;

            std::istringstream ss(line);
            HighScoreEntry e;
            if (ss >> e.name >> e.score)
                entries.push_back(e);
        }

        return entries;
    }

    void HighScoreRepository::Save(const std::filesystem::path& dataPath,
                                    const std::string& name, int score)
    {
        const auto path = FilePath(dataPath);

        std::filesystem::create_directories(path.parent_path());

        auto entries = Load(dataPath);
        entries.push_back({ name, score });

        std::sort(entries.begin(), entries.end(),
            [](const HighScoreEntry& a, const HighScoreEntry& b)
            { return a.score > b.score; });

        if (static_cast<int>(entries.size()) > k_MaxEntries)
            entries.resize(static_cast<std::size_t>(k_MaxEntries));

        std::ofstream file(path);
        for (const auto& e : entries)
            file << e.name << ' ' << e.score << '\n';
    }
}