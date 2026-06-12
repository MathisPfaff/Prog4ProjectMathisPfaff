#pragma once
#include "GameState.h"

namespace dae
{
    class PlayingState final : public GameState
    {
    public:
        PlayingState() = default;
        ~PlayingState() override = default;

        void OnEnter(GameManagerComponent* manager) override;
        void OnExit (GameManagerComponent* manager) override;
        std::unique_ptr<GameState> Update(GameManagerComponent* manager) override;
    };
}