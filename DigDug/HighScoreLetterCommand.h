#pragma once
#include "Command.h"

namespace dae
{
    class HighScoreNameEntryComponent;

    class HighScoreLetterCommand final : public Command
    {
    public:
        HighScoreLetterCommand(HighScoreNameEntryComponent* entry, int direction);
        void Execute() override;

    private:
        HighScoreNameEntryComponent* m_pEntry{};
        int                          m_Direction{};
    };
}