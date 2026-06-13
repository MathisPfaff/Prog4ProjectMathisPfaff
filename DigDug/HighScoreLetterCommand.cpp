#include "HighScoreLetterCommand.h"
#include "HighScoreNameEntryComponent.h"

namespace dae
{
    HighScoreLetterCommand::HighScoreLetterCommand(HighScoreNameEntryComponent* entry, int direction)
        : m_pEntry(entry)
        , m_Direction(direction)
    {}

    void HighScoreLetterCommand::Execute()
    {
        if (m_pEntry)
            m_pEntry->ChangeLetter(m_Direction);
    }
}