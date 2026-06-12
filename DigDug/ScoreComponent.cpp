#include "ScoreComponent.h"

namespace dae
{
    void ScoreComponent::AddScoreForKill(EnemyType type, int enemyRow, bool fygarSameRowAsPlayer)
    {
        AddScore(CalculateKillScore(type, enemyRow, fygarSameRowAsPlayer));
    }

    int ScoreComponent::GetUndergroundLevel(int row)
    {
        if (row <= 4)  return 1;   // ground (0) + underground level 1 (rows 1-4)
        if (row <= 8)  return 2;   // underground level 2
        if (row <= 12) return 3;   // underground level 3
        return 4;                  // underground level 4
    }

    int ScoreComponent::CalculateKillScore(EnemyType type, int enemyRow, bool fygarSameRowAsPlayer)
    {
        // Pooka base points per underground level
        static constexpr int pookaPoints[5] = { 0, 200, 300, 400, 500 };

        const int level = GetUndergroundLevel(enemyRow);
        int score = pookaPoints[level];

        if (type == EnemyType::Fygar)
        {
            score *= 2;                    // Fygar always gives double Pooka points
            if (fygarSameRowAsPlayer)
                score *= 2;               // fire-breath threat: double again (4× Pooka total)
        }

        return score;
    }
}