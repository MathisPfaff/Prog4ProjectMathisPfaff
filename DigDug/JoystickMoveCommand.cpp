#include "JoystickMoveCommand.h"
#include "InputManager.h"
#include "Controller.h"
#include "GameObject.h"
#include "PlayerMovementComponent.h"
#include "PumpComponent.h"
#include <glm/glm.hpp>
#include <cmath>

namespace dae
{
    JoystickMoveCommand::JoystickMoveCommand(GameObject* pPlayer, int controllerIndex)
        : GameObjectCommand(pPlayer)
        , m_ControllerIndex(controllerIndex)
    {}

    void JoystickMoveCommand::Execute()
    {
        auto* actor = GetGameObject();
        if (!actor) return;

        // Look up the controller fresh each frame – avoids stale pointer issues
        auto* ctrl = InputManager::GetInstance().GetOrCreateController(
            static_cast<unsigned int>(m_ControllerIndex));
        if (!ctrl) return;

        const glm::vec2 stick = ctrl->GetLeftStick();
        if (glm::length(stick) < 0.1f) return;   // in deadzone – do nothing

        // Snap analog direction to the nearest cardinal for grid-based movement
        glm::vec2 dir;
        if (std::abs(stick.x) >= std::abs(stick.y))
            dir = { stick.x > 0.f ? 1.f : -1.f, 0.f };
        else
            dir = { 0.f, stick.y > 0.f ? 1.f : -1.f };

        // Moving releases a stuck pump beam (same behaviour as MoveCommand)
        if (auto* pump = actor->GetComponent<PumpComponent>())
            pump->ReleaseStuck();

        if (auto* mv = actor->GetComponent<PlayerMovementComponent>())
            mv->SetDesiredDirection(dir);
    }
}