#pragma once
#include "GameState.h"
#include "GameMode.h"

namespace dae
{
    class GameObject;

    class PlayingState final : public GameState
    {
    public:
        explicit PlayingState(GameMode mode = GameMode::SinglePlayer);
        ~PlayingState() override = default;

        void OnEnter(GameManagerComponent* manager) override;
        void OnExit (GameManagerComponent* manager) override;
        std::unique_ptr<GameState> Update(GameManagerComponent* manager) override;

    private:
        GameMode m_Mode;

        void BindPlayer1Inputs(GameObject* player, bool includeController, unsigned int controllerIndex);
        void BindPlayer2Inputs(GameObject* player, unsigned int controllerIndex);
    };
}