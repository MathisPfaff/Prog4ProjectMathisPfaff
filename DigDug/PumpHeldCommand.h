#pragma once
#include "Command.h"

namespace dae
{
    class PumpHeldCommand final : public GameObjectCommand
    {
    public:
        explicit PumpHeldCommand(GameObject* pGameObject);
        void Execute() override;
    };
}