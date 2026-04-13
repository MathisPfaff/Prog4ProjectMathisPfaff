#include "Subject.h"
#include <algorithm>

void dae::Subject::AddObserver(Observer* observer)
{
	if (!observer) return;
	if (std::find(m_Observers.begin(), m_Observers.end(), observer) == m_Observers.end())
		m_Observers.push_back(observer);
}

void dae::Subject::RemoveObserver(Observer* observer)
{
	m_Observers.erase(
		std::remove(m_Observers.begin(), m_Observers.end(), observer),
		m_Observers.end());
}

void dae::Subject::Notify(BaseComponent* owner, unsigned int eventID)
{
	// Iterate a copy so observers can safely unregister during a callback
	auto observers = m_Observers;
	for (Observer* observer : observers)
		if (observer)
			observer->OnNotify(owner, eventID);
}