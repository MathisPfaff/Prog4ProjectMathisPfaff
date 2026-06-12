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
        auto* pump = GetGameObject()->GetComponent<PumpComponent>();
        if (!pump) return;

        pump->Fire();         // starts beam when idle   (no-op otherwise)
        pump->InflatePulse(); // +1.5f per press when stuck (no-op otherwise)
    }
}