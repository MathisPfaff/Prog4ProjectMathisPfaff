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

        // Call these after AddComponent to wire up the text objects
        void SetLetterText(int slot, TextComponent* text);  // slot 0..2
        void SetReadyText(TextComponent* text);

        // D-Pad Left/Right → move between the 4 slots
        void NavigateHorizontal(int delta);   // -1 left, +1 right

        // D-Pad Up/Down → cycle the letter on the currently selected letter slot
        void ChangeLetter(int delta);          // +1 next, -1 prev  (A-Z wraps)

        // True only when slot 3 (READY) is selected
        bool IsReadySelected() const { return m_SelectedSlot == 3; }

        std::string GetInitials() const;

    private:
        static constexpr int k_TotalSlots = 4;   // slots 0-2 = letters, slot 3 = READY

        std::array<char, 3>           m_Letters{ 'A', 'A', 'A' };
        int                           m_SelectedSlot{ 0 };

        std::array<TextComponent*, 3> m_pLetterTexts{ nullptr, nullptr, nullptr };
        TextComponent* m_pReadyText{ nullptr };

        void RefreshVisuals();
    };
}