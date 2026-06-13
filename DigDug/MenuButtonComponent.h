#pragma once
#include "BaseComponent.h"
#include <SDL3/SDL_pixels.h>

namespace dae
{
    class TextComponent;

    class MenuButtonComponent final : public BaseComponent
    {
    public:
        MenuButtonComponent(GameObject* owner, TextComponent* text);
        ~MenuButtonComponent() override = default;

        void FixedUpdate(float) override {}
        void Update()           override {}
        void LateUpdate()       override {}
        void Render() const     override {}

        void SetSelected(bool selected);
        bool IsSelected() const { return m_Selected; }

    private:
        TextComponent* m_pText{};
        bool           m_Selected{ false };

        static constexpr SDL_Color k_NormalColor{ 255, 255, 255, 255 };
        static constexpr SDL_Color k_SelectedColor{ 50, 220,  50, 255 };
    };
}