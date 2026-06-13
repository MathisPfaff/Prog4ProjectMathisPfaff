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
        // Make sure we always get an absolute path so ofstream can find it
        // regardless of what the current working directory is at call-time.
        return std::filesystem::absolute(dataPath) / k_FileName;
    }

    std::vector<HighScoreEntry> HighScoreRepository::Load(
        const std::filesystem::path& dataPath)
    {
        std::vector<HighScoreEntry> entries;

        std::ifstream file(FilePath(dataPath));
        if (!file.is_open())
            return entries;   // file doesn't exist yet – that's fine

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

        // Ensure the directory exists before writing
        std::filesystem::create_directories(path.parent_path());

        // Load existing entries and append the new one
        auto entries = Load(dataPath);
        entries.push_back({ name, score });

        // Sort descending by score
        std::sort(entries.begin(), entries.end(),
            [](const HighScoreEntry& a, const HighScoreEntry& b)
            { return a.score > b.score; });

        // Keep only the top N
        if (static_cast<int>(entries.size()) > k_MaxEntries)
            entries.resize(static_cast<std::size_t>(k_MaxEntries));

        // Write – one "NAME SCORE" per line
        std::ofstream file(path);
        for (const auto& e : entries)
            file << e.name << ' ' << e.score << '\n';
    }
}