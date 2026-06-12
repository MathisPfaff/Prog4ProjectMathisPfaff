#include "MoveCommand.h"
#include "GameObject.h"
#include "PlayerMovementComponent.h"
#include "PumpComponent.h"

namespace dae
{
    MoveCommand::MoveCommand(GameObject* pGameObject, glm::vec2 direction)
        : GameObjectCommand(pGameObject)
        , m_Direction(direction)
    {}

    void MoveCommand::Execute()
    {
        auto* actor = GetGameObject();
        if (!actor) return;

        // Any movement input releases a stuck pump beam
        if (auto* pump = actor->GetComponent<PumpComponent>())
            pump->ReleaseStuck();

        auto* movement = actor->GetComponent<PlayerMovementComponent>();
        if (!movement) return;
        movement->SetDesiredDirection(m_Direction);
    }
}