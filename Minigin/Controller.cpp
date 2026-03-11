#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>
#include "Controller.h"

namespace dae
{
	class Controller::ControllerImpl
	{
	public:
		explicit ControllerImpl(unsigned int controllerIndex)
			: m_ControllerIndex(controllerIndex)
		{
			ZeroMemory(&m_PreviousState, sizeof(XINPUT_STATE));
			ZeroMemory(&m_CurrentState, sizeof(XINPUT_STATE));
		}

		void Update()
		{
			m_PreviousState = m_CurrentState;
			ZeroMemory(&m_CurrentState, sizeof(XINPUT_STATE));
			XInputGetState(m_ControllerIndex, &m_CurrentState);

			const WORD buttonChanges    = m_CurrentState.Gamepad.wButtons ^ m_PreviousState.Gamepad.wButtons;
			m_ButtonsPressedThisFrame   = buttonChanges & m_CurrentState.Gamepad.wButtons;
			m_ButtonsReleasedThisFrame  = buttonChanges & (~m_CurrentState.Gamepad.wButtons);
		}

		bool IsDown(unsigned int button) const     { return (m_CurrentState.Gamepad.wButtons & button) != 0; }
		bool IsUp(unsigned int button) const       { return (m_CurrentState.Gamepad.wButtons & button) == 0; }
		bool IsPressed(unsigned int button) const  { return (m_ButtonsPressedThisFrame & button) != 0; }
		bool IsReleased(unsigned int button) const { return (m_ButtonsReleasedThisFrame & button) != 0; }

	private:
		unsigned int m_ControllerIndex;
		XINPUT_STATE m_PreviousState{};
		XINPUT_STATE m_CurrentState{};
		WORD m_ButtonsPressedThisFrame{};
		WORD m_ButtonsReleasedThisFrame{};
	};

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
#endif // WIN32