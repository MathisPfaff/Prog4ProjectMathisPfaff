#include "Observer.h"
#include "GameActor.h"
#include <algorithm>

dae::Observer::~Observer()
{
    for (GameActor* subject : m_Subjects)
    {
        if (subject)
            subject->RemoveObserver(this);
    }
    m_Subjects.clear();
}

void dae::Observer::AddSubject(GameActor* subject)
{
    if (!subject) return;

    if (std::find(m_Subjects.begin(), m_Subjects.end(), subject) == m_Subjects.end())
        m_Subjects.push_back(subject);
}

void dae::Observer::RemoveSubject(GameActor* subject)
{
    m_Subjects.erase(
        std::remove(m_Subjects.begin(), m_Subjects.end(), subject),
        m_Subjects.end());
}

void dae::Observer::OnSubjectRemoved(GameActor* subject)
{
    RemoveSubject(subject);
}