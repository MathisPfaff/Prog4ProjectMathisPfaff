#include "HighScoreState.h"
#include "GameManagerComponent.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "TextComponent.h"
#include <string>
#include <SDL3/SDL.h>

namespace dae
{
    static constexpr float kHeadlineX = 205.f;
    static constexpr float kHeadlineY = 255.f;
    static constexpr float kScoreX    = 225.f;
    static constexpr float kScoreY    = 320.f;

    HighScoreState::HighScoreState(int finalScore, bool playerWon)
        : m_FinalScore(finalScore)
        , m_PlayerWon(playerWon)
    {}

    void HighScoreState::OnEnter(GameManagerComponent* manager)
    {
        manager->ClearGameWorld();

        auto& scene = SceneManager::GetInstance().GetActiveScene();
        auto& res   = ResourceManager::GetInstance();

        auto largeFont = res.LoadFont("Lingua.otf", 40);
        auto smallFont = res.LoadFont("Lingua.otf", 28);

        // Headline: "YOU WIN!" in gold, "GAME OVER" in red
        auto headlineObj = std::make_unique<GameObject>();
        headlineObj->SetLocalPosition(kHeadlineX, kHeadlineY);
        if (m_PlayerWon)
            headlineObj->AddComponent<TextComponent>("YOU WIN!", largeFont, SDL_Color{ 255, 215, 0, 255 });
        else
            headlineObj->AddComponent<TextComponent>("GAME OVER", largeFont, SDL_Color{ 255, 50, 50, 255 });
        scene.Add(std::move(headlineObj));

        // Score line
        auto scoreObj = std::make_unique<GameObject>();
        scoreObj->SetLocalPosition(kScoreX, kScoreY);
        scoreObj->AddComponent<TextComponent>(
            "Score: " + std::to_string(m_FinalScore), smallFont, SDL_Color{ 255, 255, 255, 255 });
        scene.Add(std::move(scoreObj));
    }

    void HighScoreState::OnExit(GameManagerComponent*)
    {
        // Terminal state – nothing to clean up
    }

    std::unique_ptr<GameState> HighScoreState::Update(GameManagerComponent*)
    {
        // Terminal state – no transitions
        return nullptr;
    }
}