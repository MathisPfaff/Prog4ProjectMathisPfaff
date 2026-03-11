#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include "InputManager.h"

bool dae::InputManager::ProcessInput()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_EVENT_QUIT)
			return false;

		ImGui_ImplSDL3_ProcessEvent(&e);
	}

	ProcessKeyboardBindings();

#ifdef WIN32
	for (auto& controller : m_Controllers)
		controller->Update();
	ProcessControllerBindings();
#endif

	return true;
}

void dae::InputManager::BindKeyboard(SDL_Scancode key, KeyState state, std::unique_ptr<Command> command)
{
	m_KeyboardBindings.push_back({ key, state, std::move(command) });
}

void dae::InputManager::ClearBindings()
{
	m_KeyboardBindings.clear();
#ifdef WIN32
	m_ControllerBindings.clear();
#endif
}

void dae::InputManager::ProcessKeyboardBindings()
{
	int numKeys{};
	const bool* pCurrentState = SDL_GetKeyboardState(&numKeys);

	if (m_PreviousKeyboardState.empty())
		m_PreviousKeyboardState.assign(numKeys, 0);

	for (const auto& binding : m_KeyboardBindings)
	{
		const bool current  = pCurrentState[binding.key];
		const bool previous = static_cast<bool>(m_PreviousKeyboardState[binding.key]);

		bool shouldExecute = false;
		switch (binding.state)
		{
		case KeyState::Pressed:  shouldExecute = current && !previous;  break;
		case KeyState::Released: shouldExecute = !current && previous;  break;
		case KeyState::Held:     shouldExecute = current;               break;
		}

		if (shouldExecute)
			binding.command->Execute();
	}

	for (int i = 0; i < numKeys; ++i)
		m_PreviousKeyboardState[i] = static_cast<uint8_t>(pCurrentState[i]);
}

#ifdef WIN32
void dae::InputManager::BindController(unsigned int controllerIndex,
	Controller::ControllerButton button, KeyState state, std::unique_ptr<Command> command)
{
	GetOrCreateController(controllerIndex);
	m_ControllerBindings.push_back({ controllerIndex, button, state, std::move(command) });
}

dae::Controller* dae::InputManager::GetOrCreateController(unsigned int index)
{
	for (auto& controller : m_Controllers)
	{
		if (controller->GetIndex() == index)
			return controller.get();
	}
	m_Controllers.push_back(std::make_unique<Controller>(index));
	return m_Controllers.back().get();
}

void dae::InputManager::ProcessControllerBindings()
{
	for (const auto& binding : m_ControllerBindings)
	{
		Controller* controller = nullptr;
		for (const auto& ctrl : m_Controllers)
		{
			if (ctrl->GetIndex() == binding.controllerIndex)
			{
				controller = ctrl.get();
				break;
			}
		}
		if (!controller) continue;

		bool shouldExecute = false;
		switch (binding.state)
		{
		case KeyState::Pressed:  shouldExecute = controller->IsPressed(binding.button);  break;
		case KeyState::Released: shouldExecute = controller->IsReleased(binding.button); break;
		case KeyState::Held:     shouldExecute = controller->IsDown(binding.button);     break;
		}

		if (shouldExecute)
			binding.command->Execute();
	}
}
#endif
