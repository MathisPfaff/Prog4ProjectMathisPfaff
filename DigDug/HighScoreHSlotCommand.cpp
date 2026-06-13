#include "HighScoreHSlotCommand.h"
#include "HighScoreNameEntryComponent.h"

namespace dae
{
    HighScoreHSlotCommand::HighScoreHSlotCommand(HighScoreNameEntryComponent* entry, int direction)
        : m_pEntry(entry)
        , m_Direction(direction)
    {}

    void HighScoreHSlotCommand::Execute()
    {
        if (m_pEntry)
            m_pEntry->NavigateHorizontal(m_Direction);
    }
}