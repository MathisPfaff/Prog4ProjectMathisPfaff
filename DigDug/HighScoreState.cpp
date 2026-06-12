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
    // Window is 675 x 622 (kSidebarWidth + kGridWidth + kRightPadding, kGridOffsetY + kGridHeight + kBottomPadding)
    // These positions place the text visually centred on that canvas.
    static constexpr float kGameOverX = 205.f;
    static constexpr float kGameOverY = 255.f;
    static constexpr float kScoreX = 225.f;
    static constexpr float kScoreY = 320.f;

    HighScoreState::HighScoreState(int finalScore)
        : m_FinalScore(finalScore)
    {}

    void HighScoreState::OnEnter(GameManagerComponent* manager)
    {
        // Remove every game-world object (grid, player, enemies, HUD)
        manager->ClearGameWorld();

        auto& scene = SceneManager::GetInstance().GetActiveScene();
        auto& res = ResourceManager::GetInstance();

        auto largeFont = res.LoadFont("Lingua.otf", 40);
        auto smallFont = res.LoadFont("Lingua.otf", 28);

        // "GAME OVER" headline
        auto gameOverObj = std::make_unique<GameObject>();
        gameOverObj->SetLocalPosition(kGameOverX, kGameOverY);
        gameOverObj->AddComponent<TextComponent>("GAME OVER", largeFont, SDL_Color{ 255, 50, 50, 255 });
        scene.Add(std::move(gameOverObj));

        // Final score line
        auto scoreObj = std::make_unique<GameObject>();
        scoreObj->SetLocalPosition(kScoreX, kScoreY);
        const std::string scoreText = "Score: " + std::to_string(m_FinalScore);
        scoreObj->AddComponent<TextComponent>(scoreText, smallFont, SDL_Color{ 255, 215, 0, 255 });
        scene.Add(std::move(scoreObj));
    }

    void HighScoreState::OnExit(GameManagerComponent*)
    {
        // Terminal state for now – nothing to clean up
    }

    std::unique_ptr<GameState> HighScoreState::Update(GameManagerComponent*)
    {
        // Terminal state – no transitions
        return nullptr;
    }
}