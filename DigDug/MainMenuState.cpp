#include "MainMenuState.h"
#include "PlayingState.h"
#include "GameManagerComponent.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "TextComponent.h"
#include "InputManager.h"
#include "EnterCommand.h"
#include "Controller.h"
#include <SDL3/SDL.h>

namespace dae
{
    // Window: 675 x 622 – centre x ≈ 337
    static constexpr float kTitleX = 215.f;  static constexpr float kTitleY = 130.f;
    static constexpr float kOpt1X = 227.f;  static constexpr float kOpt1Y = 240.f;
    static constexpr float kOpt2X = 286.f;  static constexpr float kOpt2Y = 295.f;
    static constexpr float kOpt3X = 269.f;  static constexpr float kOpt3Y = 350.f;
    static constexpr float kHintX = 165.f;  static constexpr float kHintY = 460.f;

    void MainMenuState::OnEnter(GameManagerComponent* manager)
    {
        manager->ResetStartRequest();

        auto& scene = SceneManager::GetInstance().GetActiveScene();
        auto& res = ResourceManager::GetInstance();

        auto titleFont = res.LoadFont("Lingua.otf", 50);
        auto optionFont = res.LoadFont("Lingua.otf", 28);
        auto hintFont = res.LoadFont("Lingua.otf", 18);

        auto titleObj = std::make_unique<GameObject>();
        titleObj->SetLocalPosition(kTitleX, kTitleY);
        titleObj->AddComponent<TextComponent>("DIG DUG", titleFont, SDL_Color{ 255, 220, 0, 255 });
        m_pTitleObject = titleObj.get();
        scene.Add(std::move(titleObj));

        auto spObj = std::make_unique<GameObject>();
        spObj->SetLocalPosition(kOpt1X, kOpt1Y);
        spObj->AddComponent<TextComponent>("Single Player", optionFont, SDL_Color{ 255, 255, 255, 255 });
        m_pSinglePlayerObject = spObj.get();
        scene.Add(std::move(spObj));

        auto vsObj = std::make_unique<GameObject>();
        vsObj->SetLocalPosition(kOpt2X, kOpt2Y);
        vsObj->AddComponent<TextComponent>("Versus", optionFont, SDL_Color{ 255, 255, 255, 255 });
        m_pVersusObject = vsObj.get();
        scene.Add(std::move(vsObj));

        auto tpObj = std::make_unique<GameObject>();
        tpObj->SetLocalPosition(kOpt3X, kOpt3Y);
        tpObj->AddComponent<TextComponent>("2 Player", optionFont, SDL_Color{ 255, 255, 255, 255 });
        m_pTwoPlayerObject = tpObj.get();
        scene.Add(std::move(tpObj));

        auto hintObj = std::make_unique<GameObject>();
        hintObj->SetLocalPosition(kHintX, kHintY);
        hintObj->AddComponent<TextComponent>("Press Enter / Start to play", hintFont, SDL_Color{ 180, 180, 180, 255 });
        m_pHintObject = hintObj.get();
        scene.Add(std::move(hintObj));

        // Bind Enter on keyboard and controller
        auto& input = InputManager::GetInstance();
        input.BindKeyboard(SDL_SCANCODE_RETURN, KeyState::Pressed, std::make_unique<EnterCommand>(manager));
        input.BindController(0, Controller::ControllerButton::Start, KeyState::Pressed, std::make_unique<EnterCommand>(manager));
        input.BindController(0, Controller::ControllerButton::ButtonA, KeyState::Pressed, std::make_unique<EnterCommand>(manager));
    }

    void MainMenuState::OnExit(GameManagerComponent*)
    {
        if (m_pTitleObject) { m_pTitleObject->MarkForDestroy();        m_pTitleObject = nullptr; }
        if (m_pSinglePlayerObject) { m_pSinglePlayerObject->MarkForDestroy(); m_pSinglePlayerObject = nullptr; }
        if (m_pVersusObject) { m_pVersusObject->MarkForDestroy();       m_pVersusObject = nullptr; }
        if (m_pTwoPlayerObject) { m_pTwoPlayerObject->MarkForDestroy();    m_pTwoPlayerObject = nullptr; }
        if (m_pHintObject) { m_pHintObject->MarkForDestroy();         m_pHintObject = nullptr; }

        InputManager::GetInstance().ClearBindings();
    }

    std::unique_ptr<GameState> MainMenuState::Update(GameManagerComponent* manager)
    {
        if (manager->IsStartRequested())
            return std::make_unique<PlayingState>();

        return nullptr;
    }
}