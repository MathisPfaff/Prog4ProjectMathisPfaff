#include "HighScoreState.h"
#include "MainMenuState.h"
#include "GameManagerComponent.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "TextComponent.h"
#include "InputManager.h"
#include "EnterCommand.h"
#include "Controller.h"
#include <string>
#include <SDL3/SDL.h>

namespace dae
{
    static constexpr float kHeadlineX = 205.f;  static constexpr float kHeadlineY = 220.f;
    static constexpr float kScoreX    = 225.f;  static constexpr float kScoreY    = 300.f;
    static constexpr float kHintX     = 155.f;  static constexpr float kHintY     = 390.f;

    HighScoreState::HighScoreState(int finalScore, bool playerWon)
        : m_FinalScore(finalScore)
        , m_PlayerWon(playerWon)
    {}

    void HighScoreState::OnEnter(GameManagerComponent* manager)
    {
        manager->ClearGameWorld();
        manager->ResetStartRequest();

        auto& scene = SceneManager::GetInstance().GetActiveScene();
        auto& res   = ResourceManager::GetInstance();

        auto largeFont = res.LoadFont("Lingua.otf", 40);
        auto smallFont = res.LoadFont("Lingua.otf", 28);
        auto hintFont  = res.LoadFont("Lingua.otf", 18);

        // Headline
        auto headlineObj = std::make_unique<GameObject>();
        headlineObj->SetLocalPosition(kHeadlineX, kHeadlineY);
        if (m_PlayerWon)
            headlineObj->AddComponent<TextComponent>("YOU WIN!", largeFont, SDL_Color{ 255, 215,   0, 255 });
        else
            headlineObj->AddComponent<TextComponent>("GAME OVER", largeFont, SDL_Color{ 255,  50,  50, 255 });
        m_pHeadlineObject = headlineObj.get();
        scene.Add(std::move(headlineObj));

        // Score
        auto scoreObj = std::make_unique<GameObject>();
        scoreObj->SetLocalPosition(kScoreX, kScoreY);
        scoreObj->AddComponent<TextComponent>(
            "Score: " + std::to_string(m_FinalScore), smallFont, SDL_Color{ 255, 255, 255, 255 });
        m_pScoreObject = scoreObj.get();
        scene.Add(std::move(scoreObj));

        // Return hint
        auto hintObj = std::make_unique<GameObject>();
        hintObj->SetLocalPosition(kHintX, kHintY);
        hintObj->AddComponent<TextComponent>("Press Enter / Start to return to menu", hintFont, SDL_Color{ 180, 180, 180, 255 });
        m_pHintObject = hintObj.get();
        scene.Add(std::move(hintObj));

        // Bind Enter
        auto& input = InputManager::GetInstance();
        input.BindKeyboard(SDL_SCANCODE_RETURN, KeyState::Pressed, std::make_unique<EnterCommand>(manager));
        input.BindController(0, Controller::ControllerButton::Start,   KeyState::Pressed, std::make_unique<EnterCommand>(manager));
        input.BindController(0, Controller::ControllerButton::ButtonA, KeyState::Pressed, std::make_unique<EnterCommand>(manager));
    }

    void HighScoreState::OnExit(GameManagerComponent*)
    {
        if (m_pHeadlineObject) { m_pHeadlineObject->MarkForDestroy(); m_pHeadlineObject = nullptr; }
        if (m_pScoreObject)    { m_pScoreObject->MarkForDestroy();    m_pScoreObject    = nullptr; }
        if (m_pHintObject)     { m_pHintObject->MarkForDestroy();     m_pHintObject     = nullptr; }

        InputManager::GetInstance().ClearBindings();
    }

    std::unique_ptr<GameState> HighScoreState::Update(GameManagerComponent* manager)
    {
        if (manager->IsStartRequested())
            return std::make_unique<MainMenuState>();

        return nullptr;
    }
}