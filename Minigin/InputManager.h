#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <memory>
#include <cstdint>
#include "Singleton.h"
#include "Command.h"
#include "Controller.h"

namespace dae
{
    enum class KeyState { Pressed, Released, Held };

    class InputManager final : public Singleton<InputManager>
    {
    public:
        bool ProcessInput();

        // ── Button / key bindings ─────────────────────────────────────────────
        void BindKeyboard  (SDL_Scancode key, KeyState state, std::unique_ptr<Command> command);
        void BindController(unsigned int controllerIndex, Controller::ControllerButton button,
                            KeyState state, std::unique_ptr<Command> command);

        void UnbindKeyboard  (SDL_Scancode key, KeyState state);
        void UnbindController(unsigned int controllerIndex, Controller::ControllerButton button,
                              KeyState state);

        // ── Axis binding (left thumbstick) ────────────────────────────────────
        void BindControllerLeftStick(unsigned int controllerIndex, std::unique_ptr<Command> command);

        // Clears ALL bindings (button + axis)
        void ClearBindings();

        // Returns (or creates) the controller for the given index.
        Controller* GetOrCreateController(unsigned int index);

        // Returns true if a physical controller is plugged into the given XInput slot.
        bool IsControllerConnected(unsigned int index);

    private:
        enum class BindingType { Keyboard, Controller };

        struct InputBinding
        {
            std::unique_ptr<Command> command;
            BindingType type{};
            KeyState    state{};
            SDL_Scancode key{};
            int          controllerIndex{};
            Controller::ControllerButton button{};
        };

        struct AxisBinding
        {
            std::unique_ptr<Command> command;
        };

        bool IsTriggered(const InputBinding& binding) const;

        std::vector<InputBinding>             m_Bindings;
        std::vector<AxisBinding>              m_AxisBindings;
        std::vector<uint8_t>                  m_PreviousKeyboardState;
        std::vector<std::unique_ptr<Controller>> m_Controllers;
    };
}
