#include "NavigateMenuCommand.h"
#include "MenuNavigationComponent.h"

namespace dae
{
    NavigateMenuCommand::NavigateMenuCommand(MenuNavigationComponent* nav, int direction)
        : m_pNav(nav)
        , m_Direction(direction)
    {}

    void NavigateMenuCommand::Execute()
    {
        if (m_pNav)
            m_pNav->Navigate(m_Direction);
    }
}