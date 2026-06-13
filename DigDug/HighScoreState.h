#pragma once
#include "GameState.h"

namespace dae
{
    class GameObject;
    class HighScoreNameEntryComponent;

    class HighScoreState final : public GameState
    {
    public:
        explicit HighScoreState(int finalScore, bool playerWon);
        ~HighScoreState() override = default;

        void OnEnter(GameManagerComponent* manager) override;
        void OnExit (GameManagerComponent* manager) override;
        std::unique_ptr<GameState> Update(GameManagerComponent* manager) override;

    private:
        int  m_FinalScore{};
        bool m_PlayerWon{};

        // Headline / score / hint
        GameObject* m_pHeadlineObject{};
        GameObject* m_pScoreObject{};
        GameObject* m_pHintObject{};

        // Name-entry row  (3 letter slots + READY)
        GameObject* m_pLetter0Object{};
        GameObject* m_pLetter1Object{};
        GameObject* m_pLetter2Object{};
        GameObject* m_pReadyObject{};

        // Navigation logic object
        GameObject*                  m_pEntryNavObject{};
        HighScoreNameEntryComponent* m_pEntryNav{};
    };
}