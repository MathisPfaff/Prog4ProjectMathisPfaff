#pragma once
#include "Command.h"

namespace dae
{
    class MuteCommand final : public Command
    {
    public:
        void Execute() override;
    };
}