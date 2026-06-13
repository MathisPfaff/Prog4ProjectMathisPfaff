#pragma once
#include "BaseComponent.h"
#include <vector>

namespace dae
{
    class MenuButtonComponent;

    class MenuNavigationComponent final : public BaseComponent
    {
    public:
        explicit MenuNavigationComponent(GameObject* owner);
        ~MenuNavigationComponent() override = default;

        void FixedUpdate(float) override {}
        void Update()           override {}
        void LateUpdate()       override {}
        void Render() const     override {}

        // Registers a button; the first one added is auto-selected
        void AddButton(MenuButtonComponent* button);

        // delta: -1 = previous, +1 = next, wraps around
        void Navigate(int delta);

        int GetSelectedIndex() const { return m_SelectedIndex; }

    private:
        std::vector<MenuButtonComponent*> m_Buttons{};
        int m_SelectedIndex{ 0 };

        void SelectIndex(int index);
    };
}