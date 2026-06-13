#include "PlayingState.h"
#include "HighScoreState.h"
#include "GameManagerComponent.h"
#include "InputManager.h"
#include "MoveCommand.h"
#include "PumpCommand.h"
#include "PumpHeldCommand.h"
#include "JoystickMoveCommand.h"
#include "Controller.h"
#include <glm/glm.hpp>
#include <SDL3/SDL.h>

namespace dae
{
    PlayingState::PlayingState(GameMode mode)
        : m_Mode(mode)
    {}

    void PlayingState::OnEnter(GameManagerComponent* manager)
    {
        manager->SpawnGrid();
        manager->SpawnPooka(3, 1);
        manager->SpawnPooka(9, 9);
        manager->SpawnFygar(9, 5);

        switch (m_Mode)
        {
        case GameMode::SinglePlayer:
        case GameMode::Versus:
            manager->SpawnPlayer(1, 1);
            BindPlayer1Inputs(manager->GetPlayer());
            break;

        case GameMode::TwoPlayer:
            manager->SpawnPlayer (1,  1);
            manager->SpawnPlayer2(12, 1);
            BindPlayer1Inputs(manager->GetPlayer());
            BindPlayer2Inputs(manager->GetPlayer2());
            break;
        }
    }

    void PlayingState::BindPlayer1Inputs(GameObject* player)
    {
        if (!player) return;
        auto& input = InputManager::GetInstance();

        // Keyboard WASD + Space
        input.BindKeyboard(SDL_SCANCODE_W,     KeyState::Held,    std::make_unique<MoveCommand>(player, glm::vec2{  0.f, -1.f }));
        input.BindKeyboard(SDL_SCANCODE_S,     KeyState::Held,    std::make_unique<MoveCommand>(player, glm::vec2{  0.f,  1.f }));
        input.BindKeyboard(SDL_SCANCODE_A,     KeyState::Held,    std::make_unique<MoveCommand>(player, glm::vec2{ -1.f,  0.f }));
        input.BindKeyboard(SDL_SCANCODE_D,     KeyState::Held,    std::make_unique<MoveCommand>(player, glm::vec2{  1.f,  0.f }));
        input.BindKeyboard(SDL_SCANCODE_SPACE, KeyState::Pressed, std::make_unique<PumpCommand>(player));
        input.BindKeyboard(SDL_SCANCODE_SPACE, KeyState::Held,    std::make_unique<PumpHeldCommand>(player));

        // Controller 0 – left joystick (NOT DPad) + Button A
        input.BindControllerLeftStick(0, std::make_unique<JoystickMoveCommand>(player, 0));
        input.BindController(0, Controller::ControllerButton::ButtonA, KeyState::Pressed, std::make_unique<PumpCommand>(player));
        input.BindController(0, Controller::ControllerButton::ButtonA, KeyState::Held,    std::make_unique<PumpHeldCommand>(player));
    }

    void PlayingState::BindPlayer2Inputs(GameObject* player)
    {
        if (!player) return;
        auto& input = InputManager::GetInstance();

        // Controller 1 – left joystick + Button A
        input.BindControllerLeftStick(1, std::make_unique<JoystickMoveCommand>(player, 1));
        input.BindController(1, Controller::ControllerButton::ButtonA, KeyState::Pressed, std::make_unique<PumpCommand>(player));
        input.BindController(1, Controller::ControllerButton::ButtonA, KeyState::Held,    std::make_unique<PumpHeldCommand>(player));
    }

    void PlayingState::OnExit(GameManagerComponent*)
    {
        InputManager::GetInstance().ClearBindings();
    }

    std::unique_ptr<GameState> PlayingState::Update(GameManagerComponent* manager)
    {
        if (manager->IsGameOver())
            return std::make_unique<HighScoreState>(manager->GetFinalScore(), false);

        if (manager->IsPlayerWon())
            return std::make_unique<HighScoreState>(manager->GetFinalScore(), true);

        return nullptr;
    }
}