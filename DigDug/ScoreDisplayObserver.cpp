#include "ScoreDisplayObserver.h"
#include "TextComponent.h"
#include <string>

namespace dae
{
    ScoreDisplayObserver::ScoreDisplayObserver(ScoreComponent* scoreComp, TextComponent* textComp)
        : m_ScoreComp(scoreComp)
        , m_TextComp(textComp)
    {
        if (m_ScoreComp)
            m_ScoreComp->AddObserver(this);
        UpdateDisplay();
    }

    void ScoreDisplayObserver::OnNotify(BaseComponent*, unsigned int eventID)
    {
        if (eventID == ScoreEvent::Added)
            UpdateDisplay();
    }

    void ScoreDisplayObserver::UpdateDisplay()
    {
        if (!m_ScoreComp || !m_TextComp) return;
        m_TextComp->SetText("Score: " + std::to_string(m_ScoreComp->GetScore()));
    }
}