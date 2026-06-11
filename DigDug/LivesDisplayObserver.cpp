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
        {
            m_HealthComp->AddObserver(this);
        }
        UpdateDisplay();
    }

    void LivesDisplayObserver::OnNotify(BaseComponent*, unsigned int eventID)
    {
        if (eventID == HealthEvent::Changed || eventID == HealthEvent::Died)
        {
            UpdateDisplay();
        }
    }

    void LivesDisplayObserver::UpdateDisplay()
    {
        if (!m_HealthComp || !m_TextComp) return;

        const int lives = m_HealthComp->GetHealth();
        const std::string text = "Lives: " + std::to_string(lives);
        m_TextComp->SetText(text);
    }
}