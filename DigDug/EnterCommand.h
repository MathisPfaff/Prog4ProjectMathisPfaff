#pragma once
#include "Command.h"

namespace dae
{
    class GameManagerComponent;

    class EnterCommand final : public Command
    {
    public:
        explicit EnterCommand(GameManagerComponent* manager);
        void Execute() override;

    private:
        GameManagerComponent* m_pManager{};
    };
}