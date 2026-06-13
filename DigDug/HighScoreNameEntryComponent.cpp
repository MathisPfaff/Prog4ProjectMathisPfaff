#include "HighScoreNameEntryComponent.h"
#include "TextComponent.h"
#include <SDL3/SDL_pixels.h>
#include <string>

namespace dae
{
    static constexpr SDL_Color k_Normal{ 255, 255, 255, 255 };
    static constexpr SDL_Color k_Selected{ 50, 220,  50, 255 };

    HighScoreNameEntryComponent::HighScoreNameEntryComponent(GameObject* owner)
        : BaseComponent(owner)
    {}

    void HighScoreNameEntryComponent::SetLetterText(int slot, TextComponent* text)
    {
        if (slot >= 0 && slot < 3)
            m_pLetterTexts[slot] = text;
        RefreshVisuals();
    }

    void HighScoreNameEntryComponent::SetReadyText(TextComponent* text)
    {
        m_pReadyText = text;
        RefreshVisuals();
    }

    void HighScoreNameEntryComponent::NavigateHorizontal(int delta)
    {
        m_SelectedSlot = (m_SelectedSlot + delta + k_TotalSlots) % k_TotalSlots;
        RefreshVisuals();
    }

    void HighScoreNameEntryComponent::ChangeLetter(int delta)
    {
        if (m_SelectedSlot < 0 || m_SelectedSlot >= 3)
            return;   // on READY slot – nothing to cycle

        int letter = m_Letters[m_SelectedSlot] - 'A';
        letter = (letter + delta + 26) % 26;
        m_Letters[m_SelectedSlot] = static_cast<char>('A' + letter);

        if (m_pLetterTexts[m_SelectedSlot])
            m_pLetterTexts[m_SelectedSlot]->SetText(std::string(1, m_Letters[m_SelectedSlot]));
    }

    std::string HighScoreNameEntryComponent::GetInitials() const
    {
        return std::string{ m_Letters[0], m_Letters[1], m_Letters[2] };
    }

    void HighScoreNameEntryComponent::RefreshVisuals()
    {
        for (int i = 0; i < 3; ++i)
        {
            if (m_pLetterTexts[i])
                m_pLetterTexts[i]->SetColor(m_SelectedSlot == i ? k_Selected : k_Normal);
        }
        if (m_pReadyText)
            m_pReadyText->SetColor(m_SelectedSlot == 3 ? k_Selected : k_Normal);
    }
}