#pragma once
#include <memory>

namespace dae
{
	class Controller final
	{
	public:
		enum class ControllerButton : unsigned int
		{
			DPadUp        = 0x0001,
			DPadDown      = 0x0002,
			DPadLeft      = 0x0004,
			DPadRight     = 0x0008,
			Start         = 0x0010,
			Back          = 0x0020,
			LeftThumb     = 0x0040,
			RightThumb    = 0x0080,
			LeftShoulder  = 0x0100,
			RightShoulder = 0x0200,
			ButtonA       = 0x1000,
			ButtonB       = 0x2000,
			ButtonX       = 0x4000,
			ButtonY       = 0x8000
		};

		explicit Controller(unsigned int controllerIndex);
		~Controller();

		Controller(const Controller&) = delete;
		Controller& operator=(const Controller&) = delete;

		void Update();
		bool IsDown(ControllerButton button) const;
		bool IsUp(ControllerButton button) const;
		bool IsPressed(ControllerButton button) const;
		bool IsReleased(ControllerButton button) const;

		unsigned int GetIndex() const { return m_ControllerIndex; }

	private:
		class ControllerImpl;
		unsigned int m_ControllerIndex;
		std::unique_ptr<ControllerImpl> m_pImpl;
	};
}