#include "MenuNavigationComponent.h"
#include "MenuButtonComponent.h"

namespace dae
{
    MenuNavigationComponent::MenuNavigationComponent(GameObject* owner)
        : BaseComponent(owner)
    {}

    void MenuNavigationComponent::AddButton(MenuButtonComponent* button)
    {
        if (!button) return;
        m_Buttons.push_back(button);
        if (m_Buttons.size() == 1)
            SelectIndex(0);
    }

    void MenuNavigationComponent::Navigate(int delta)
    {
        if (m_Buttons.empty()) return;
        const int count = static_cast<int>(m_Buttons.size());
        SelectIndex((m_SelectedIndex + delta + count) % count);
    }

    void MenuNavigationComponent::SelectIndex(int index)
    {
        const int count = static_cast<int>(m_Buttons.size());
        if (count == 0) return;

        if (m_SelectedIndex >= 0 && m_SelectedIndex < count)
            m_Buttons[m_SelectedIndex]->SetSelected(false);

        m_SelectedIndex = index;
        if (m_SelectedIndex >= 0 && m_SelectedIndex < count)
            m_Buttons[m_SelectedIndex]->SetSelected(true);
    }
}