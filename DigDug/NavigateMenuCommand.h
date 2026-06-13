#pragma once
#include "Command.h"

namespace dae
{
    class MenuNavigationComponent;

    class NavigateMenuCommand final : public Command
    {
    public:
        NavigateMenuCommand(MenuNavigationComponent* nav, int direction);
        void Execute() override;

    private:
        MenuNavigationComponent* m_pNav{};
        int                      m_Direction{};
    };
}