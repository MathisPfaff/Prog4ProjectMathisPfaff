#pragma once
#include "Command.h"

namespace dae
{
    class HighScoreNameEntryComponent;

    // Moves the highlighted slot left or right (D-Pad Left / Right)
    class HighScoreHSlotCommand final : public Command
    {
    public:
        HighScoreHSlotCommand(HighScoreNameEntryComponent* entry, int direction);
        void Execute() override;

    private:
        HighScoreNameEntryComponent* m_pEntry{};
        int                          m_Direction{};
    };
}