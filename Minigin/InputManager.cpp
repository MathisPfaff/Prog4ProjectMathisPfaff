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

	int numKeys{};
	SDL_GetKeyboardState(&numKeys);
	if (m_PreviousKeyboardState.empty())
		m_PreviousKeyboardState.assign(numKeys, 0);

	for (auto& ctrl : m_Controllers)
		ctrl->Update();

	for (auto& binding : m_Bindings)
		if (binding.IsTriggered())
			binding.command->Execute();

	const bool* pCurrentState = SDL_GetKeyboardState(nullptr);
	for (int i = 0; i < numKeys; ++i)
		m_PreviousKeyboardState[i] = static_cast<uint8_t>(pCurrentState[i]);

	return true;
}

void dae::InputManager::BindKeyboard(SDL_Scancode key, KeyState state, std::unique_ptr<Command> command)
{
	m_Bindings.push_back(
	{
		[this, key, state]() -> bool
		{
			const bool current  = SDL_GetKeyboardState(nullptr)[key];
			const bool previous = !m_PreviousKeyboardState.empty() &&
			                      static_cast<bool>(m_PreviousKeyboardState[key]);
			switch (state)
			{
			case KeyState::Pressed:  return current && !previous;
			case KeyState::Released: return !current && previous;
			case KeyState::Held:     return current;
			}
			return false;
		},
		std::move(command)
	});
}

void dae::InputManager::BindController(unsigned int controllerIndex,
	Controller::ControllerButton button, KeyState state, std::unique_ptr<Command> command)
{
	Controller* ctrl = GetOrCreateController(controllerIndex);
	m_Bindings.push_back(
	{
		[ctrl, button, state]() -> bool
		{
			switch (state)
			{
			case KeyState::Pressed:  return ctrl->IsPressed(button);
			case KeyState::Released: return ctrl->IsReleased(button);
			case KeyState::Held:     return ctrl->IsDown(button);
			}
			return false;
		},
		std::move(command)
	});
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

void dae::InputManager::ClearBindings()
{
	m_Bindings.clear();
}
