#pragma once
#include "GameState.h"
#include <vector>

namespace dae
{
    class GameObject;

    class ShowHighScoreState final : public GameState
    {
    public:
        ShowHighScoreState() = default;
        ~ShowHighScoreState() override = default;

        void OnEnter(GameManagerComponent* manager) override;
        void OnExit(GameManagerComponent* manager) override;
        std::unique_ptr<GameState> Update(GameManagerComponent* manager) override;

    private:
        GameObject* m_pTitleObject{};
        GameObject* m_pHintObject{};
        std::vector<GameObject*> m_pEntryObjects{};
    };
}