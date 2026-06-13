#include "MuteCommand.h"
#include "ServiceLocator.h"

namespace dae
{
    void MuteCommand::Execute()
    {
        auto& ss = ServiceLocator::GetSoundSystem();
        ss.SetMuted(!ss.IsMuted());
    }
}