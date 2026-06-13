#include "EnterCommand.h"
#include "GameManagerComponent.h"

namespace dae
{
    EnterCommand::EnterCommand(GameManagerComponent* manager)
        : m_pManager(manager)
    {}

    void EnterCommand::Execute()
    {
        if (m_pManager)
            m_pManager->RequestStart();
    }
}