#include "MainMenuState.h"
#include "PlayingState.h"
#include "GameManagerComponent.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "TextComponent.h"
#include "MenuButtonComponent.h"
#include "MenuNavigationComponent.h"
#include "InputManager.h"
#include "EnterCommand.h"
#include "NavigateMenuCommand.h"
#include "Controller.h"
#include <SDL3/SDL.h>

namespace dae
{
    static constexpr float kTitleX = 215.f;  static constexpr float kTitleY = 130.f;
    static constexpr float kOpt1X  = 227.f;  static constexpr float kOpt1Y  = 240.f;
    static constexpr float kOpt2X  = 286.f;  static constexpr float kOpt2Y  = 295.f;
    static constexpr float kOpt3X  = 269.f;  static constexpr float kOpt3Y  = 350.f;
    static constexpr float kHintX  = 130.f;  static constexpr float kHintY  = 460.f;

    void MainMenuState::OnEnter(GameManagerComponent* manager)
    {
        manager->ResetStartRequest();

        auto& scene = SceneManager::GetInstance().GetActiveScene();
        auto& res   = ResourceManager::GetInstance();

        auto titleFont  = res.LoadFont("Lingua.otf", 50);
        auto optionFont = res.LoadFont("Lingua.otf", 28);
        auto hintFont   = res.LoadFont("Lingua.otf", 18);

        // ── Title ─────────────────────────────────────────────────────────────
        auto titleObj = std::make_unique<GameObject>();
        titleObj->SetLocalPosition(kTitleX, kTitleY);
        titleObj->AddComponent<TextComponent>("DIG DUG", titleFont, SDL_Color{ 255, 220, 0, 255 });
        m_pTitleObject = titleObj.get();
        scene.Add(std::move(titleObj));

        // ── Option buttons ────────────────────────────────────────────────────
        auto spObj  = std::make_unique<GameObject>();
        spObj->SetLocalPosition(kOpt1X, kOpt1Y);
        auto* spText = spObj->AddComponent<TextComponent>("Single Player", optionFont, SDL_Color{ 255, 255, 255, 255 });
        auto* spBtn  = spObj->AddComponent<MenuButtonComponent>(spText);
        m_pSinglePlayerObject = spObj.get();
        scene.Add(std::move(spObj));

        auto vsObj  = std::make_unique<GameObject>();
        vsObj->SetLocalPosition(kOpt2X, kOpt2Y);
        auto* vsText = vsObj->AddComponent<TextComponent>("Versus", optionFont, SDL_Color{ 255, 255, 255, 255 });
        auto* vsBtn  = vsObj->AddComponent<MenuButtonComponent>(vsText);
        m_pVersusObject = vsObj.get();
        scene.Add(std::move(vsObj));

        auto tpObj  = std::make_unique<GameObject>();
        tpObj->SetLocalPosition(kOpt3X, kOpt3Y);
        auto* tpText = tpObj->AddComponent<TextComponent>("2 Player", optionFont, SDL_Color{ 255, 255, 255, 255 });
        auto* tpBtn  = tpObj->AddComponent<MenuButtonComponent>(tpText);
        m_pTwoPlayerObject = tpObj.get();
        scene.Add(std::move(tpObj));

        // ── Hint ──────────────────────────────────────────────────────────────
        auto hintObj = std::make_unique<GameObject>();
        hintObj->SetLocalPosition(kHintX, kHintY);
        hintObj->AddComponent<TextComponent>("Use arrows / DPad to navigate, Enter / Start to play",
                                             hintFont, SDL_Color{ 180, 180, 180, 255 });
        m_pHintObject = hintObj.get();
        scene.Add(std::move(hintObj));

        // ── Navigation controller ─────────────────────────────────────────────
        auto navObj    = std::make_unique<GameObject>();
        m_pMenuNav     = navObj->AddComponent<MenuNavigationComponent>();
        m_pMenuNav->AddButton(spBtn);   // index 0 – auto-selected (green)
        m_pMenuNav->AddButton(vsBtn);   // index 1
        m_pMenuNav->AddButton(tpBtn);   // index 2
        m_pMenuNavObject = navObj.get();
        scene.Add(std::move(navObj));

        // ── Input bindings ────────────────────────────────────────────────────
        auto& input = InputManager::GetInstance();

        // Confirm
        input.BindKeyboard(SDL_SCANCODE_RETURN, KeyState::Pressed,
                           std::make_unique<EnterCommand>(manager));
        input.BindController(0, Controller::ControllerButton::Start,   KeyState::Pressed,
                             std::make_unique<EnterCommand>(manager));
        input.BindController(0, Controller::ControllerButton::ButtonA, KeyState::Pressed,
                             std::make_unique<EnterCommand>(manager));

        // Navigate – keyboard
        input.BindKeyboard(SDL_SCANCODE_UP,    KeyState::Pressed,
                           std::make_unique<NavigateMenuCommand>(m_pMenuNav, -1));
        input.BindKeyboard(SDL_SCANCODE_DOWN,  KeyState::Pressed,
                           std::make_unique<NavigateMenuCommand>(m_pMenuNav, +1));
        input.BindKeyboard(SDL_SCANCODE_LEFT,  KeyState::Pressed,
                           std::make_unique<NavigateMenuCommand>(m_pMenuNav, -1));
        input.BindKeyboard(SDL_SCANCODE_RIGHT, KeyState::Pressed,
                           std::make_unique<NavigateMenuCommand>(m_pMenuNav, +1));

        // Navigate – controller DPad
        input.BindController(0, Controller::ControllerButton::DPadUp,    KeyState::Pressed,
                             std::make_unique<NavigateMenuCommand>(m_pMenuNav, -1));
        input.BindController(0, Controller::ControllerButton::DPadDown,  KeyState::Pressed,
                             std::make_unique<NavigateMenuCommand>(m_pMenuNav, +1));
        input.BindController(0, Controller::ControllerButton::DPadLeft,  KeyState::Pressed,
                             std::make_unique<NavigateMenuCommand>(m_pMenuNav, -1));
        input.BindController(0, Controller::ControllerButton::DPadRight, KeyState::Pressed,
                             std::make_unique<NavigateMenuCommand>(m_pMenuNav, +1));
    }

    void MainMenuState::OnExit(GameManagerComponent*)
    {
        // Clear bindings FIRST – destroys NavigateMenuCommand/EnterCommand objects
        // while the nav component is still alive (just marked for destroy)
        InputManager::GetInstance().ClearBindings();

        if (m_pTitleObject)        { m_pTitleObject->MarkForDestroy();        m_pTitleObject        = nullptr; }
        if (m_pSinglePlayerObject) { m_pSinglePlayerObject->MarkForDestroy(); m_pSinglePlayerObject = nullptr; }
        if (m_pVersusObject)       { m_pVersusObject->MarkForDestroy();       m_pVersusObject       = nullptr; }
        if (m_pTwoPlayerObject)    { m_pTwoPlayerObject->MarkForDestroy();    m_pTwoPlayerObject    = nullptr; }
        if (m_pHintObject)         { m_pHintObject->MarkForDestroy();         m_pHintObject         = nullptr; }
        if (m_pMenuNavObject)      { m_pMenuNavObject->MarkForDestroy();      m_pMenuNavObject      = nullptr; }
        m_pMenuNav = nullptr;
    }

    std::unique_ptr<GameState> MainMenuState::Update(GameManagerComponent* manager)
    {
        if (!manager->IsStartRequested())
            return nullptr;

        GameMode mode = GameMode::SinglePlayer;
        if (m_pMenuNav)
        {
            switch (m_pMenuNav->GetSelectedIndex())
            {
            case 0: mode = GameMode::SinglePlayer; break;
            case 1: mode = GameMode::Versus;       break;
            case 2: mode = GameMode::TwoPlayer;    break;
            default: break;
            }
        }
        return std::make_unique<PlayingState>(mode);
    }
}