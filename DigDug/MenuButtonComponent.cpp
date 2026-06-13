#include "MenuButtonComponent.h"
#include "TextComponent.h"

namespace dae
{
    MenuButtonComponent::MenuButtonComponent(GameObject* owner, TextComponent* text)
        : BaseComponent(owner)
        , m_pText(text)
    {}

    void MenuButtonComponent::SetSelected(bool selected)
    {
        m_Selected = selected;
        if (m_pText)
            m_pText->SetColor(selected ? k_SelectedColor : k_NormalColor);
    }
}