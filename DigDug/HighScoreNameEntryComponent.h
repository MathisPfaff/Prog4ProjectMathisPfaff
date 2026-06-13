#pragma once
#include "BaseComponent.h"
#include <array>
#include <string>

namespace dae
{
    class TextComponent;

    class HighScoreNameEntryComponent final : public BaseComponent
    {
    public:
        explicit HighScoreNameEntryComponent(GameObject* owner);
        ~HighScoreNameEntryComponent() override = default;

        void FixedUpdate(float) override {}
        void Update()           override {}
        void LateUpdate()       override {}
        void Render() const     override {}

        void SetLetterText(int slot, TextComponent* text);
        void SetReadyText(TextComponent* text);

        void NavigateHorizontal(int delta);

        void ChangeLetter(int delta);

        bool IsReadySelected() const { return m_SelectedSlot == 3; }

        std::string GetInitials() const;

    private:
        static constexpr int k_TotalSlots = 4;

        std::array<char, 3>           m_Letters{ 'A', 'A', 'A' };
        int                           m_SelectedSlot{ 0 };

        std::array<TextComponent*, 3> m_pLetterTexts{ nullptr, nullptr, nullptr };
        TextComponent* m_pReadyText{ nullptr };

        void RefreshVisuals();
    };
}