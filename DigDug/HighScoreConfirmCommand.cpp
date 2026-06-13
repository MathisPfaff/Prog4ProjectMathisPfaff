#include "HighScoreConfirmCommand.h"
#include "HighScoreNameEntryComponent.h"
#include "GameManagerComponent.h"

namespace dae
{
    HighScoreConfirmCommand::HighScoreConfirmCommand(HighScoreNameEntryComponent* entry,
        GameManagerComponent* manager)
        : m_pEntry(entry)
        , m_pManager(manager)
    {}

    void HighScoreConfirmCommand::Execute()
    {
        if (m_pEntry && m_pEntry->IsReadySelected() && m_pManager)
            m_pManager->RequestStart();
    }
}