#pragma once
#include "GameState.h"

namespace dae
{
    class GameObject;
    class HighScoreNameEntryComponent;

    class HighScoreState final : public GameState
    {
    public:
        explicit HighScoreState(int finalScore, bool playerWon, bool showEntry = true);
        ~HighScoreState() override = default;

        void OnEnter(GameManagerComponent* manager) override;
        void OnExit (GameManagerComponent* manager) override;
        std::unique_ptr<GameState> Update(GameManagerComponent* manager) override;

    private:
        int  m_FinalScore{};
        bool m_PlayerWon{};
        bool m_ShowEntry{};
        bool m_AlreadySaved{ false };

        GameObject* m_pHeadlineObject{};
        GameObject* m_pScoreObject{};
        GameObject* m_pHintObject{};

        GameObject* m_pLetter0Object{};
        GameObject* m_pLetter1Object{};
        GameObject* m_pLetter2Object{};
        GameObject* m_pReadyObject{};

        GameObject*                  m_pEntryNavObject{};
        HighScoreNameEntryComponent* m_pEntryNav{};
    };
}