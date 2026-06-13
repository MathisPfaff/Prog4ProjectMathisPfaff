#pragma once
#include "Command.h"

namespace dae
{
    class HighScoreNameEntryComponent;
    class GameManagerComponent;

    // Signals the manager to start (go to main menu) only when READY is selected.
    class HighScoreConfirmCommand final : public Command
    {
    public:
        HighScoreConfirmCommand(HighScoreNameEntryComponent* entry,
                                GameManagerComponent*        manager);
        void Execute() override;

    private:
        HighScoreNameEntryComponent* m_pEntry{};
        GameManagerComponent*        m_pManager{};
    };
}