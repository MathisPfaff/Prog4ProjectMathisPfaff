#pragma once
#include "GameState.h"

namespace dae
{
    class GameObject;
    class MenuNavigationComponent;

    class MainMenuState final : public GameState
    {
    public:
        MainMenuState() = default;
        ~MainMenuState() override = default;

        void OnEnter(GameManagerComponent* manager) override;
        void OnExit (GameManagerComponent* manager) override;
        std::unique_ptr<GameState> Update(GameManagerComponent* manager) override;

    private:
        GameObject*              m_pTitleObject{};
        GameObject*              m_pSinglePlayerObject{};
        GameObject*              m_pVersusObject{};
        GameObject*              m_pTwoPlayerObject{};
        GameObject*              m_pHintObject{};
        GameObject*              m_pMenuNavObject{};
        MenuNavigationComponent* m_pMenuNav{};
    };
}