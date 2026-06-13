#pragma once
#include "Command.h"

namespace dae
{
    class JoystickMoveCommand final : public GameObjectCommand
    {
    public:
        JoystickMoveCommand(GameObject* pPlayer, int controllerIndex);
        void Execute() override;

    private:
        int m_ControllerIndex{};
    };
}