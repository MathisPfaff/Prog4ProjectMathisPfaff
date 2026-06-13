#include "LivesDisplayObserver.h"
#include "TextComponent.h"
#include "Hash.h"
#include <string>

namespace dae
{
    LivesDisplayObserver::LivesDisplayObserver(HealthComponent* healthComp, TextComponent* textComp)
        : m_HealthComp(healthComp)
        , m_TextComp(textComp)
    {
        if (m_HealthComp)
            m_HealthComp->AddObserver(this);
        UpdateDisplay();
    }

    LivesDisplayObserver::~LivesDisplayObserver()
    {
        // Do NOT call RemoveObserver here.
        // During normal gameplay cleanup, ClearGameWorld() calls ClearReferences()
        // explicitly while the subject is still alive.
        // At shutdown, the player (and its HealthComponent/Subject) is already
        // destroyed before this destructor runs (scene destroys objects forward),
        // so touching m_HealthComp here would be a use-after-free.
    }

    void LivesDisplayObserver::OnNotify(BaseComponent*, unsigned int eventID)
    {
        if (eventID == HealthEvent::Changed || eventID == HealthEvent::Died)
            UpdateDisplay();
    }

    void LivesDisplayObserver::ClearReferences()
    {
        if (m_HealthComp)
        {
            m_HealthComp->RemoveObserver(this);
            m_HealthComp = nullptr;
        }
        m_TextComp = nullptr;
    }

    void LivesDisplayObserver::UpdateDisplay()
    {
        if (!m_HealthComp || !m_TextComp) return;

        const int lives = m_HealthComp->GetHealth();
        const std::string text = "Lives: " + std::to_string(lives);
        m_TextComp->SetText(text);
    }
}