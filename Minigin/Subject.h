#pragma once
#include <vector>
#include "Observer.h"

namespace dae
{
	class BaseComponent;

	class Subject final
	{
	public:
		Subject() = default;
		~Subject() = default;
		Subject(const Subject&)            = delete;
		Subject(Subject&&)                 = delete;
		Subject& operator=(const Subject&) = delete;
		Subject& operator=(Subject&&)      = delete;

		void AddObserver(Observer* observer);
		void RemoveObserver(Observer* observer);
		void Notify(BaseComponent* owner, unsigned int eventID);

	private:
		std::vector<Observer*> m_Observers;
	};
}