#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <memory>
#include <cstdint>
#include "Singleton.h"
#include "Command.h"

#ifdef WIN32
#include "Controller.h"
#endif

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

#ifdef WIN32
		void BindController(unsigned int controllerIndex, Controller::ControllerButton button,
		                    KeyState state, std::unique_ptr<Command> command);
#endif

		void ClearBindings();

	private:
		struct KeyboardBinding
		{
			SDL_Scancode key;
			KeyState state;
			std::unique_ptr<Command> command;
		};

		std::vector<KeyboardBinding> m_KeyboardBindings;
		std::vector<uint8_t> m_PreviousKeyboardState;

		void ProcessKeyboardBindings();

#ifdef WIN32
		struct ControllerBinding
		{
			unsigned int controllerIndex;
			Controller::ControllerButton button;
			KeyState state;
			std::unique_ptr<Command> command;
		};

		std::vector<ControllerBinding> m_ControllerBindings;
		std::vector<std::unique_ptr<Controller>> m_Controllers;

		Controller* GetOrCreateController(unsigned int index);
		void ProcessControllerBindings();
#endif
	};
}
