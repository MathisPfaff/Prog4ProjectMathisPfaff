#pragma once
#include "Command.h"

namespace dae
{
    class HighScoreNameEntryComponent;
    class GameManagerComponent;

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