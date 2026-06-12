#pragma once
#include "Command.h"

namespace dae
{
    class PumpCommand final : public GameObjectCommand
    {
    public:
        explicit PumpCommand(GameObject* pGameObject);
        void Execute() override;
    };
}