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

        void AddButton(MenuButtonComponent* button);

        void Navigate(int delta);

        int GetSelectedIndex() const { return m_SelectedIndex; }

    private:
        std::vector<MenuButtonComponent*> m_Buttons{};
        int m_SelectedIndex{ 0 };

        void SelectIndex(int index);
    };
}