#include "PlayingState.h"
#include "HighScoreState.h"
#include "GameManagerComponent.h"
#include "InputManager.h"
#include "MoveCommand.h"
#include "PumpCommand.h"
#include "PumpHeldCommand.h"
#include "Controller.h"
#include <glm/glm.hpp>
#include <SDL3/SDL.h>

namespace dae
{
    void PlayingState::OnEnter(GameManagerComponent* manager)
    {
        manager->SpawnPlayer(1, 1);
        manager->SpawnPooka (3, 1);
        manager->SpawnPooka (9, 9);
        manager->SpawnFygar (9, 5);

        auto* player = manager->GetPlayer();
        if (!player) return;

        auto& input = InputManager::GetInstance();

        // Keyboard – WASD + Space
        input.BindKeyboard(SDL_SCANCODE_W,     KeyState::Held,    std::make_unique<MoveCommand>(player, glm::vec2{  0.f, -1.f }));
        input.BindKeyboard(SDL_SCANCODE_S,     KeyState::Held,    std::make_unique<MoveCommand>(player, glm::vec2{  0.f,  1.f }));
        input.BindKeyboard(SDL_SCANCODE_A,     KeyState::Held,    std::make_unique<MoveCommand>(player, glm::vec2{ -1.f,  0.f }));
        input.BindKeyboard(SDL_SCANCODE_D,     KeyState::Held,    std::make_unique<MoveCommand>(player, glm::vec2{  1.f,  0.f }));
        input.BindKeyboard(SDL_SCANCODE_SPACE, KeyState::Pressed, std::make_unique<PumpCommand>(player));
        input.BindKeyboard(SDL_SCANCODE_SPACE, KeyState::Held,    std::make_unique<PumpHeldCommand>(player));

        // Controller 0 – DPad + Button A
        input.BindController(0, Controller::ControllerButton::DPadUp,    KeyState::Held,    std::make_unique<MoveCommand>(player, glm::vec2{  0.f, -1.f }));
        input.BindController(0, Controller::ControllerButton::DPadDown,  KeyState::Held,    std::make_unique<MoveCommand>(player, glm::vec2{  0.f,  1.f }));
        input.BindController(0, Controller::ControllerButton::DPadLeft,  KeyState::Held,    std::make_unique<MoveCommand>(player, glm::vec2{ -1.f,  0.f }));
        input.BindController(0, Controller::ControllerButton::DPadRight, KeyState::Held,    std::make_unique<MoveCommand>(player, glm::vec2{  1.f,  0.f }));
        input.BindController(0, Controller::ControllerButton::ButtonA,   KeyState::Pressed, std::make_unique<PumpCommand>(player));
        input.BindController(0, Controller::ControllerButton::ButtonA,   KeyState::Held,    std::make_unique<PumpHeldCommand>(player));
    }

    void PlayingState::OnExit(GameManagerComponent*)
    {
        InputManager::GetInstance().ClearBindings();
    }

    std::unique_ptr<GameState> PlayingState::Update(GameManagerComponent* manager)
    {
        if (manager->IsGameOver())
            return std::make_unique<HighScoreState>(manager->GetFinalScore());

        return nullptr;
    }
}