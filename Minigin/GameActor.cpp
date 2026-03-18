#include "GameActor.h"
#include "Observer.h"
#include <algorithm>

dae::GameActor::GameActor(BaseComponent* owner)
    : m_Owner(owner)
{
}

dae::GameActor::~GameActor()
{
    // Tell all observers this subject is gone to prevent dangling pointers
    for (Observer* observer : m_Observers)
    {
        if (observer)
            observer->OnSubjectRemoved(this);
    }
    m_Observers.clear();
}

void dae::GameActor::AddObserver(Observer* observer)
{
    if (!observer) return;

    if (std::find(m_Observers.begin(), m_Observers.end(), observer) == m_Observers.end())
    {
        m_Observers.push_back(observer);
        observer->AddSubject(this);
    }
}

void dae::GameActor::RemoveObserver(Observer* observer)
{
    m_Observers.erase(
        std::remove(m_Observers.begin(), m_Observers.end(), observer),
        m_Observers.end());
}

void dae::GameActor::Notify(Observer::Event event)
{
    // Iterate a copy so observers can safely unregister during a callback
    auto observers = m_Observers;
    for (Observer* observer : observers)
    {
        if (observer)
            observer->OnNotify(m_Owner, event);
    }
}