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

        pump->Fire();         
        pump->InflatePulse();
    }
}