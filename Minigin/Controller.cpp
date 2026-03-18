#include "Controller.h"

// ── Platform-specific implementation ─────────────────────────────────────────

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>

namespace dae
{
	class Controller::ControllerImpl
	{
	public:
		explicit ControllerImpl(unsigned int controllerIndex)
			: m_ControllerIndex(controllerIndex)
		{
			ZeroMemory(&m_PreviousState, sizeof(XINPUT_STATE));
			ZeroMemory(&m_CurrentState,  sizeof(XINPUT_STATE));
		}

		void Update()
		{
			m_PreviousState = m_CurrentState;
			ZeroMemory(&m_CurrentState, sizeof(XINPUT_STATE));
			XInputGetState(m_ControllerIndex, &m_CurrentState);

			const WORD buttonChanges   = m_CurrentState.Gamepad.wButtons ^ m_PreviousState.Gamepad.wButtons;
			m_ButtonsPressedThisFrame  = buttonChanges &  m_CurrentState.Gamepad.wButtons;
			m_ButtonsReleasedThisFrame = buttonChanges & ~m_CurrentState.Gamepad.wButtons;
		}

		bool IsDown(unsigned int button) const     { return (m_CurrentState.Gamepad.wButtons  & button) != 0; }
		bool IsUp(unsigned int button) const       { return (m_CurrentState.Gamepad.wButtons  & button) == 0; }
		bool IsPressed(unsigned int button) const  { return (m_ButtonsPressedThisFrame        & button) != 0; }
		bool IsReleased(unsigned int button) const { return (m_ButtonsReleasedThisFrame       & button) != 0; }

	private:
		unsigned int m_ControllerIndex;
		XINPUT_STATE m_PreviousState{};
		XINPUT_STATE m_CurrentState{};
		WORD m_ButtonsPressedThisFrame{};
		WORD m_ButtonsReleasedThisFrame{};
	};
}

#elif defined(__EMSCRIPTEN__)
#include <SDL3/SDL.h>

namespace dae
{
	class Controller::ControllerImpl
	{
	public:
		explicit ControllerImpl(unsigned int controllerIndex)
			: m_ControllerIndex(controllerIndex)
		{
		}

		~ControllerImpl()
		{
			if (m_pGamepad)
				SDL_CloseGamepad(m_pGamepad);
		}

		void Update()
		{
			m_ButtonsPressedThisFrame  = 0;
			m_ButtonsReleasedThisFrame = 0;

			int count = 0;
			SDL_JoystickID* pIDs = SDL_GetGamepads(&count);

			if (!pIDs || static_cast<int>(m_ControllerIndex) >= count)
			{
				SDL_free(pIDs);
				if (m_pGamepad) { SDL_CloseGamepad(m_pGamepad); m_pGamepad = nullptr; }
				m_PreviousButtons = 0;
				m_CurrentButtons  = 0;
				return;
			}

			const SDL_JoystickID id = pIDs[m_ControllerIndex];
			SDL_free(pIDs);

			if (!m_pGamepad || SDL_GetGamepadID(m_pGamepad) != id)
			{
				if (m_pGamepad) SDL_CloseGamepad(m_pGamepad);
				m_pGamepad = SDL_OpenGamepad(id);
			}

			if (!m_pGamepad)
				return;

			m_PreviousButtons = m_CurrentButtons;
			m_CurrentButtons  = 0;

			for (const auto& mapping : k_ButtonMap)
			{
				if (SDL_GetGamepadButton(m_pGamepad, mapping.sdlButton))
					m_CurrentButtons |= mapping.mask;
			}

			const unsigned int changes = m_CurrentButtons ^ m_PreviousButtons;
			m_ButtonsPressedThisFrame  = changes &  m_CurrentButtons;
			m_ButtonsReleasedThisFrame = changes & ~m_CurrentButtons;
		}

		bool IsDown(unsigned int button) const     { return (m_CurrentButtons          & button) != 0; }
		bool IsUp(unsigned int button) const       { return (m_CurrentButtons          & button) == 0; }
		bool IsPressed(unsigned int button) const  { return (m_ButtonsPressedThisFrame & button) != 0; }
		bool IsReleased(unsigned int button) const { return (m_ButtonsReleasedThisFrame & button) != 0; }

	private:
		struct ButtonMapping { SDL_GamepadButton sdlButton; unsigned int mask; };

		static constexpr ButtonMapping k_ButtonMap[] =
		{
			{ SDL_GAMEPAD_BUTTON_SOUTH,          0x1000 }, // ButtonA
			{ SDL_GAMEPAD_BUTTON_EAST,           0x2000 }, // ButtonB
			{ SDL_GAMEPAD_BUTTON_WEST,           0x4000 }, // ButtonX
			{ SDL_GAMEPAD_BUTTON_NORTH,          0x8000 }, // ButtonY
			{ SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,  0x0100 }, // LeftShoulder
			{ SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, 0x0200 }, // RightShoulder
			{ SDL_GAMEPAD_BUTTON_BACK,           0x0020 }, // Back
			{ SDL_GAMEPAD_BUTTON_START,          0x0010 }, // Start
			{ SDL_GAMEPAD_BUTTON_LEFT_STICK,     0x0040 }, // LeftThumb
			{ SDL_GAMEPAD_BUTTON_RIGHT_STICK,    0x0080 }, // RightThumb
			{ SDL_GAMEPAD_BUTTON_DPAD_UP,        0x0001 }, // DPadUp
			{ SDL_GAMEPAD_BUTTON_DPAD_DOWN,      0x0002 }, // DPadDown
			{ SDL_GAMEPAD_BUTTON_DPAD_LEFT,      0x0004 }, // DPadLeft
			{ SDL_GAMEPAD_BUTTON_DPAD_RIGHT,     0x0008 }, // DPadRight
		};

		unsigned int m_ControllerIndex;
		SDL_Gamepad* m_pGamepad{ nullptr };
		unsigned int m_PreviousButtons{};
		unsigned int m_CurrentButtons{};
		unsigned int m_ButtonsPressedThisFrame{};
		unsigned int m_ButtonsReleasedThisFrame{};
	};
}

#endif

// ── Platform-independent forwarding ──────────────────────────────────────────

namespace dae
{
	Controller::Controller(unsigned int controllerIndex)
		: m_ControllerIndex(controllerIndex)
		, m_pImpl(std::make_unique<ControllerImpl>(controllerIndex))
	{
	}

	Controller::~Controller() = default;

	void Controller::Update()                                  { m_pImpl->Update(); }
	bool Controller::IsDown(ControllerButton button) const     { return m_pImpl->IsDown(static_cast<unsigned int>(button)); }
	bool Controller::IsUp(ControllerButton button) const       { return m_pImpl->IsUp(static_cast<unsigned int>(button)); }
	bool Controller::IsPressed(ControllerButton button) const  { return m_pImpl->IsPressed(static_cast<unsigned int>(button)); }
	bool Controller::IsReleased(ControllerButton button) const { return m_pImpl->IsReleased(static_cast<unsigned int>(button)); }
}