#include "PlayingState.h"
#include "HighScoreState.h"
#include "MainMenuState.h"
#include "GameManagerComponent.h"
#include "InputManager.h"
#include "MoveCommand.h"
#include "PumpCommand.h"
#include "PumpHeldCommand.h"
#include "MuteCommand.h"
#include "JoystickMoveCommand.h"
#include "Controller.h"
#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <memory>


namespace dae
{
    PlayingState::PlayingState(GameMode mode)
        : m_Mode(mode)
    {}

    void PlayingState::OnEnter(GameManagerComponent* manager)
    {
        manager->SpawnGrid();

        // Enemies – one per pre-dug tunnel, spread across all depth zones
        manager->SpawnPooka(2,  2);   // yellow zone,      horizontal tunnel row 2
        manager->SpawnPooka(10, 6);   // orange zone,      horizontal tunnel row 6
        manager->SpawnPooka(2,  10);  // dark-orange zone, vertical tunnel col 2
        manager->SpawnFygar(6,  13);  // dark-red zone,    horizontal tunnel row 13

        auto& input = InputManager::GetInstance();

        switch (m_Mode)
        {
        case GameMode::SinglePlayer:
        case GameMode::Versus:
            manager->SpawnPlayer(6, 7);   // centre of the player tunnel
            BindPlayer1Inputs(manager->GetPlayer(),
                              input.IsControllerConnected(0), 0);
            break;

        case GameMode::TwoPlayer:
        {
            manager->SpawnPlayer (6, 7);  // P1 – left half of the centre tunnel
            manager->SpawnPlayer2(7, 7);  // P2 – right half of the centre tunnel

            const bool ctrl0 = input.IsControllerConnected(0);
            const bool ctrl1 = input.IsControllerConnected(1);

            if (ctrl1)
            {
                BindPlayer1Inputs(manager->GetPlayer(),  /*includeController=*/true, 0);
                BindPlayer2Inputs(manager->GetPlayer2(), 1);
            }
            else if (ctrl0)
            {
                BindPlayer1Inputs(manager->GetPlayer(),  /*includeController=*/false, 0);
                BindPlayer2Inputs(manager->GetPlayer2(), 0);
            }
            else
            {
                BindPlayer1Inputs(manager->GetPlayer(),  /*includeController=*/false, 0);
            }
            break;
        }
        }

        input.BindKeyboard(SDL_SCANCODE_F2, KeyState::Pressed, std::make_unique<MuteCommand>());
    }

    void PlayingState::BindPlayer1Inputs(GameObject* player, bool includeController, unsigned int controllerIndex)
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

        if (includeController)
        {
            input.BindControllerLeftStick(controllerIndex, std::make_unique<JoystickMoveCommand>(player, controllerIndex));
            input.BindController(controllerIndex, Controller::ControllerButton::ButtonA, KeyState::Pressed, std::make_unique<PumpCommand>(player));
            input.BindController(controllerIndex, Controller::ControllerButton::ButtonA, KeyState::Held,    std::make_unique<PumpHeldCommand>(player));
        }
    }

    void PlayingState::BindPlayer2Inputs(GameObject* player, unsigned int controllerIndex)
    {
        if (!player) return;
        auto& input = InputManager::GetInstance();

        input.BindControllerLeftStick(controllerIndex, std::make_unique<JoystickMoveCommand>(player, controllerIndex));
        input.BindController(controllerIndex, Controller::ControllerButton::ButtonA, KeyState::Pressed, std::make_unique<PumpCommand>(player));
        input.BindController(controllerIndex, Controller::ControllerButton::ButtonA, KeyState::Held,    std::make_unique<PumpHeldCommand>(player));
    }

    void PlayingState::OnExit(GameManagerComponent*)
    {
        InputManager::GetInstance().ClearBindings();
    }

    std::unique_ptr<GameState> PlayingState::Update(GameManagerComponent* manager)
    {
        if (manager->IsGameOver() || manager->IsPlayerWon())
            return std::make_unique<HighScoreState>(manager->GetFinalScore(), manager->IsPlayerWon());

        return nullptr;
    }
}