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

    ScoreDisplayObserver::~ScoreDisplayObserver()
    {
        // Same reasoning as LivesDisplayObserver: do not call RemoveObserver here.
        // ClearGameWorld() calls ClearReferences() explicitly while ScoreComponent
        // is still alive. At shutdown, ScoreComponent is already destroyed first.
    }

    void ScoreDisplayObserver::OnNotify(BaseComponent*, unsigned int eventID)
    {
        if (eventID == ScoreEvent::Added)
            UpdateDisplay();
    }

    void ScoreDisplayObserver::ClearReferences()
    {
        if (m_ScoreComp)
        {
            m_ScoreComp->RemoveObserver(this);
            m_ScoreComp = nullptr;
        }
        m_TextComp = nullptr;
    }

    void ScoreDisplayObserver::UpdateDisplay()
    {
        if (!m_ScoreComp || !m_TextComp) return;
        m_TextComp->SetText("Score: " + std::to_string(m_ScoreComp->GetScore()));
    }
}