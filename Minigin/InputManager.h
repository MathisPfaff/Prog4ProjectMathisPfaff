#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <memory>
#include <cstdint>
#include <functional>
#include "Singleton.h"
#include "Command.h"
#include "Controller.h"

namespace dae
{
	enum class KeyState
	{
		Pressed,
		Released,
		Held
	};

	class InputManager final : public Singleton<InputManager>
	{
	public:
		bool ProcessInput();

		void BindKeyboard(SDL_Scancode key, KeyState state, std::unique_ptr<Command> command);
		void BindController(unsigned int controllerIndex, Controller::ControllerButton button,
		                    KeyState state, std::unique_ptr<Command> command);

		void ClearBindings();

	private:
		struct InputBinding
		{
			std::function<bool()> IsTriggered;
			std::unique_ptr<Command> command;
		};

		std::vector<InputBinding>           m_Bindings;
		std::vector<uint8_t>                m_PreviousKeyboardState;
		std::vector<std::unique_ptr<Controller>> m_Controllers;

		Controller* GetOrCreateController(unsigned int index);
	};
}
