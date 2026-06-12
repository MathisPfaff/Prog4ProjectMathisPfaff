#include "PumpHeldCommand.h"
#include "PumpComponent.h"
#include "GameObject.h"

namespace dae
{
    PumpHeldCommand::PumpHeldCommand(GameObject* pGameObject)
        : GameObjectCommand(pGameObject)
    {}

    void PumpHeldCommand::Execute()
    {
        if (auto* pump = GetGameObject()->GetComponent<PumpComponent>())
            pump->PumpHeld();
    }
}