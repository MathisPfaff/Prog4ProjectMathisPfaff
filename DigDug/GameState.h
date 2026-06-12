#pragma once
#include <memory>

namespace dae
{
    class GameManagerComponent;

    class GameState
    {
    public:
        virtual ~GameState() = default;

        virtual void OnEnter(GameManagerComponent* manager) = 0;
        virtual void OnExit (GameManagerComponent* manager) = 0;

        // Returns the next state, or nullptr to stay. Mirrors PookaState::Update.
        virtual std::unique_ptr<GameState> Update(GameManagerComponent* manager) = 0;
    };
}