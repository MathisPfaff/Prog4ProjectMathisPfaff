#pragma once
#include "Command.h"

namespace dae
{
    // Stateless command – toggles the global sound mute on/off.
    class MuteCommand final : public Command
    {
    public:
        void Execute() override;
    };
}