#pragma once

namespace dae
{
	class BaseComponent;

	class Observer
	{
	public:
		virtual ~Observer() = default;
		virtual void OnNotify(BaseComponent* entity, unsigned int eventID) = 0;
	};
}