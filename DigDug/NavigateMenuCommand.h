#pragma once
#include "Command.h"

namespace dae
{
    class MenuNavigationComponent;

    class NavigateMenuCommand final : public Command
    {
    public:
        // direction: -1 = up/previous, +1 = down/next
        NavigateMenuCommand(MenuNavigationComponent* nav, int direction);
        void Execute() override;

    private:
        MenuNavigationComponent* m_pNav{};
        int                      m_Direction{};
    };
}