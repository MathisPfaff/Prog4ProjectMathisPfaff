#include "PumpCommand.h"
#include "PumpComponent.h"
#include "GameObject.h"

namespace dae
{
    PumpCommand::PumpCommand(GameObject* pGameObject)
        : GameObjectCommand(pGameObject)
    {}

    void PumpCommand::Execute()
    {
        if (auto* pump = GetGameObject()->GetComponent<PumpComponent>())
            pump->Fire();
    }
}