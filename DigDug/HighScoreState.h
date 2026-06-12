#pragma once
#include "GameState.h"

namespace dae
{
    class HighScoreState final : public GameState
    {
    public:
        explicit HighScoreState(int finalScore);
        ~HighScoreState() override = default;

        void OnEnter(GameManagerComponent* manager) override;
        void OnExit(GameManagerComponent* manager) override;
        std::unique_ptr<GameState> Update(GameManagerComponent* manager) override;

    private:
        int m_FinalScore{};
    };
}