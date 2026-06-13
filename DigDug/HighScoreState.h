#pragma once
#include "GameState.h"

namespace dae
{
    class GameObject;
    class HighScoreNameEntryComponent;

    class HighScoreState final : public GameState
    {
    public:
        // showEntry = true  → single-player: name input + save + leaderboard
        // showEntry = false → other modes:   score only, press enter → main menu
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

        // Headline / score / hint
        GameObject* m_pHeadlineObject{};
        GameObject* m_pScoreObject{};
        GameObject* m_pHintObject{};

        // Name-entry row  (3 letter slots + READY) – single-player only
        GameObject* m_pLetter0Object{};
        GameObject* m_pLetter1Object{};
        GameObject* m_pLetter2Object{};
        GameObject* m_pReadyObject{};

        // Navigation logic object – single-player only
        GameObject*                  m_pEntryNavObject{};
        HighScoreNameEntryComponent* m_pEntryNav{};
    };
}