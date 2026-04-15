#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include "InputManager.h"

namespace dae
{
	bool InputManager::ProcessInput()
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
			if (IsTriggered(binding))
				binding.command->Execute();

		const bool* pCurrentState = SDL_GetKeyboardState(nullptr);
		for (int i = 0; i < numKeys; ++i)
			m_PreviousKeyboardState[i] = static_cast<uint8_t>(pCurrentState[i]);

		return true;
	}

	bool InputManager::IsTriggered(const InputBinding& binding) const
	{
		if (binding.type == BindingType::Keyboard)
		{
			const bool current = SDL_GetKeyboardState(nullptr)[binding.key];
			const bool previous = !m_PreviousKeyboardState.empty() &&
			                      static_cast<bool>(m_PreviousKeyboardState[binding.key]);
			switch (binding.state)
			{
			case KeyState::Pressed:  return current && !previous;
			case KeyState::Released: return !current && previous;
			case KeyState::Held:     return current;
			}
		}
		else
		{
			auto it = std::ranges::find_if(m_Controllers, [&](const auto& c)
			{
				return c->GetIndex() == static_cast<unsigned int>(binding.controllerIndex);
			});
			if (it == m_Controllers.end()) return false;
			Controller* ctrl = it->get();
			switch (binding.state)
			{
			case KeyState::Pressed:  return ctrl->IsPressed(binding.button);
			case KeyState::Released: return ctrl->IsReleased(binding.button);
			case KeyState::Held:     return ctrl->IsDown(binding.button);
			}
		}
		return false;
	}

	void InputManager::BindKeyboard(SDL_Scancode key, KeyState state, std::unique_ptr<Command> command)
	{
		InputBinding binding;
		binding.type = BindingType::Keyboard;
		binding.key = key;
		binding.state = state;
		binding.command = std::move(command);
		m_Bindings.push_back(std::move(binding));
	}

	void InputManager::BindController(unsigned int controllerIndex,
		Controller::ControllerButton button, KeyState state, std::unique_ptr<Command> command)
	{
		GetOrCreateController(controllerIndex);
		InputBinding binding;
		binding.type = BindingType::Controller;
		binding.controllerIndex = static_cast<int>(controllerIndex);
		binding.button = button;
		binding.state = state;
		binding.command = std::move(command);
		m_Bindings.push_back(std::move(binding));
	}

	void InputManager::UnbindKeyboard(SDL_Scancode key, KeyState state)
	{
		std::erase_if(m_Bindings, [key, state](const InputBinding& b)
		{
			return b.type == BindingType::Keyboard && b.key == key && b.state == state;
		});
	}

	void InputManager::UnbindController(unsigned int controllerIndex,
		Controller::ControllerButton button, KeyState state)
	{
		std::erase_if(m_Bindings, [controllerIndex, button, state](const InputBinding& b)
		{
			return b.type == BindingType::Controller &&
			       b.controllerIndex == static_cast<int>(controllerIndex) &&
			       b.button == button &&
			       b.state == state;
		});
	}

	Controller* InputManager::GetOrCreateController(unsigned int index)
	{
		for (auto& controller : m_Controllers)
		{
			if (controller->GetIndex() == index)
				return controller.get();
		}
		m_Controllers.push_back(std::make_unique<Controller>(index));
		return m_Controllers.back().get();
	}

	void InputManager::ClearBindings()
	{
		m_Bindings.clear();
	}
}
