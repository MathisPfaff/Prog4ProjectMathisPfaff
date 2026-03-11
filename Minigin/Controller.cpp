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

#elif defined(__EMSCRIPTEN__)  // ── Emscripten / web via HTML5 Gamepad API ───

#include <emscripten/html5.h>

namespace dae
{
	class Controller::ControllerImpl
	{
	public:
		explicit ControllerImpl(unsigned int controllerIndex)
			: m_ControllerIndex(controllerIndex)
		{
		}

		void Update()
		{
			m_ButtonsPressedThisFrame  = 0;
			m_ButtonsReleasedThisFrame = 0;

			emscripten_sample_gamepad_data();

			EmscriptenGamepadEvent state;
			const EMSCRIPTEN_RESULT result =
				emscripten_get_gamepad_status(static_cast<int>(m_ControllerIndex), &state);

			if (result != EMSCRIPTEN_RESULT_SUCCESS || !state.connected)
			{
				m_PreviousButtons = 0;
				m_CurrentButtons  = 0;
				return;
			}

			m_PreviousButtons = m_CurrentButtons;
			m_CurrentButtons  = 0;

			for (int i = 0; i < state.numButtons && i < 16; ++i)
			{
				if (state.digitalButton[i])
					m_CurrentButtons |= ToXInputMask(i);
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
		// Maps W3C Standard Gamepad button indices → XInput-style bitmasks
		static unsigned int ToXInputMask(int index)
		{
			switch (index)
			{
			case  0: return 0x1000; // ButtonA       (South / Cross)
			case  1: return 0x2000; // ButtonB       (East  / Circle)
			case  2: return 0x4000; // ButtonX       (West  / Square)
			case  3: return 0x8000; // ButtonY       (North / Triangle)
			case  4: return 0x0100; // LeftShoulder  (L1)
			case  5: return 0x0200; // RightShoulder (R1)
			// 6 = L2 analog, 7 = R2 analog — no XInput digital mapping
			case  8: return 0x0020; // Back
			case  9: return 0x0010; // Start
			case 10: return 0x0040; // LeftThumb
			case 11: return 0x0080; // RightThumb
			case 12: return 0x0001; // DPadUp
			case 13: return 0x0002; // DPadDown
			case 14: return 0x0004; // DPadLeft
			case 15: return 0x0008; // DPadRight
			default: return 0;
			}
		}

		unsigned int m_ControllerIndex;
		unsigned int m_PreviousButtons{};
		unsigned int m_CurrentButtons{};
		unsigned int m_ButtonsPressedThisFrame{};
		unsigned int m_ButtonsReleasedThisFrame{};
	};
}

#else  // ── No-op stub for any other unsupported platform ────────────────────

namespace dae
{
	class Controller::ControllerImpl
	{
	public:
		explicit ControllerImpl(unsigned int) {}
		void Update() {}
		bool IsDown(unsigned int) const     { return false; }
		bool IsUp(unsigned int) const       { return true;  }
		bool IsPressed(unsigned int) const  { return false; }
		bool IsReleased(unsigned int) const { return false; }
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