#include "HighScoreState.h"
#include "ShowHighScoreState.h"
#include "GameManagerComponent.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "TextComponent.h"
#include "InputManager.h"
#include "HighScoreNameEntryComponent.h"
#include "HighScoreHSlotCommand.h"
#include "HighScoreLetterCommand.h"
#include "HighScoreConfirmCommand.h"
#include "HighScoreRepository.h"
#include "Controller.h"
#include <string>
#include <SDL3/SDL.h>

namespace dae
{
    // ── layout ───────────────────────────────────────────────────────────────
    static constexpr float kHeadlineX = 205.f;  static constexpr float kHeadlineY = 220.f;
    static constexpr float kScoreX    = 225.f;  static constexpr float kScoreY    = 300.f;
    static constexpr float kHintX     = 100.f;  static constexpr float kHintY     = 355.f;

    static constexpr float kLetterY   = 400.f;
    static constexpr float kLetter0X  = 180.f;
    static constexpr float kLetter1X  = 240.f;
    static constexpr float kLetter2X  = 300.f;
    static constexpr float kReadyX    = 440.f;  static constexpr float kReadyY = 400.f;

    // ─────────────────────────────────────────────────────────────────────────
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

        auto largeFont  = res.LoadFont("Lingua.otf", 40);
        auto smallFont  = res.LoadFont("Lingua.otf", 28);
        auto letterFont = res.LoadFont("Lingua.otf", 36);
        auto readyFont  = res.LoadFont("Lingua.otf", 24);
        auto hintFont   = res.LoadFont("Lingua.otf", 16);

        // ── Headline ─────────────────────────────────────────────────────────
        {
            auto obj = std::make_unique<GameObject>();
            obj->SetLocalPosition(kHeadlineX, kHeadlineY);
            if (m_PlayerWon)
                obj->AddComponent<TextComponent>("YOU WIN!",  largeFont, SDL_Color{ 255, 215,  0, 255 });
            else
                obj->AddComponent<TextComponent>("GAME OVER", largeFont, SDL_Color{ 255,  50, 50, 255 });
            m_pHeadlineObject = obj.get();
            scene.Add(std::move(obj));
        }

        // ── Score ─────────────────────────────────────────────────────────────
        {
            auto obj = std::make_unique<GameObject>();
            obj->SetLocalPosition(kScoreX, kScoreY);
            obj->AddComponent<TextComponent>(
                "Score: " + std::to_string(m_FinalScore), smallFont, SDL_Color{ 255, 255, 255, 255 });
            m_pScoreObject = obj.get();
            scene.Add(std::move(obj));
        }

        // ── Hint ──────────────────────────────────────────────────────────────
        {
            auto obj = std::make_unique<GameObject>();
            obj->SetLocalPosition(kHintX, kHintY);
            obj->AddComponent<TextComponent>(
                "Enter initials   Left/Right: select   Up/Down: change letter   A: confirm",
                hintFont, SDL_Color{ 180, 180, 180, 255 });
            m_pHintObject = obj.get();
            scene.Add(std::move(obj));
        }

        // ── Letter slots ──────────────────────────────────────────────────────
        TextComponent* pLetterText[3]{};

        {
            auto obj = std::make_unique<GameObject>();
            obj->SetLocalPosition(kLetter0X, kLetterY);
            pLetterText[0] = obj->AddComponent<TextComponent>("A", letterFont, SDL_Color{ 255, 255, 255, 255 });
            m_pLetter0Object = obj.get();
            scene.Add(std::move(obj));
        }
        {
            auto obj = std::make_unique<GameObject>();
            obj->SetLocalPosition(kLetter1X, kLetterY);
            pLetterText[1] = obj->AddComponent<TextComponent>("A", letterFont, SDL_Color{ 255, 255, 255, 255 });
            m_pLetter1Object = obj.get();
            scene.Add(std::move(obj));
        }
        {
            auto obj = std::make_unique<GameObject>();
            obj->SetLocalPosition(kLetter2X, kLetterY);
            pLetterText[2] = obj->AddComponent<TextComponent>("A", letterFont, SDL_Color{ 255, 255, 255, 255 });
            m_pLetter2Object = obj.get();
            scene.Add(std::move(obj));
        }

        // ── READY (far right) ─────────────────────────────────────────────────
        TextComponent* pReadyText{};
        {
            auto obj = std::make_unique<GameObject>();
            obj->SetLocalPosition(kReadyX, kReadyY);
            pReadyText = obj->AddComponent<TextComponent>("READY", readyFont, SDL_Color{ 255, 255, 255, 255 });
            m_pReadyObject = obj.get();
            scene.Add(std::move(obj));
        }

        // ── Navigation component ──────────────────────────────────────────────
        {
            auto obj = std::make_unique<GameObject>();
            m_pEntryNav = obj->AddComponent<HighScoreNameEntryComponent>();
            m_pEntryNav->SetLetterText(0, pLetterText[0]);
            m_pEntryNav->SetLetterText(1, pLetterText[1]);
            m_pEntryNav->SetLetterText(2, pLetterText[2]);
            m_pEntryNav->SetReadyText(pReadyText);
            m_pEntryNavObject = obj.get();
            scene.Add(std::move(obj));
        }

        // ── Input bindings ────────────────────────────────────────────────────
        auto& input = InputManager::GetInstance();

        // Left/Right → move selected slot
        input.BindKeyboard(SDL_SCANCODE_LEFT,  KeyState::Pressed,
            std::make_unique<HighScoreHSlotCommand>(m_pEntryNav, -1));
        input.BindKeyboard(SDL_SCANCODE_RIGHT, KeyState::Pressed,
            std::make_unique<HighScoreHSlotCommand>(m_pEntryNav, +1));
        input.BindController(0, Controller::ControllerButton::DPadLeft,  KeyState::Pressed,
            std::make_unique<HighScoreHSlotCommand>(m_pEntryNav, -1));
        input.BindController(0, Controller::ControllerButton::DPadRight, KeyState::Pressed,
            std::make_unique<HighScoreHSlotCommand>(m_pEntryNav, +1));

        // Up/Down → cycle letter on current slot
        input.BindKeyboard(SDL_SCANCODE_UP,   KeyState::Pressed,
            std::make_unique<HighScoreLetterCommand>(m_pEntryNav, +1));
        input.BindKeyboard(SDL_SCANCODE_DOWN, KeyState::Pressed,
            std::make_unique<HighScoreLetterCommand>(m_pEntryNav, -1));
        input.BindController(0, Controller::ControllerButton::DPadUp,   KeyState::Pressed,
            std::make_unique<HighScoreLetterCommand>(m_pEntryNav, +1));
        input.BindController(0, Controller::ControllerButton::DPadDown, KeyState::Pressed,
            std::make_unique<HighScoreLetterCommand>(m_pEntryNav, -1));

        // Confirm → sets IsStartRequested; Update() handles the save
        input.BindKeyboard(SDL_SCANCODE_RETURN, KeyState::Pressed,
            std::make_unique<HighScoreConfirmCommand>(m_pEntryNav, manager));
        input.BindController(0, Controller::ControllerButton::Start,   KeyState::Pressed,
            std::make_unique<HighScoreConfirmCommand>(m_pEntryNav, manager));
        input.BindController(0, Controller::ControllerButton::ButtonA, KeyState::Pressed,
            std::make_unique<HighScoreConfirmCommand>(m_pEntryNav, manager));
    }

    void HighScoreState::OnExit(GameManagerComponent*)
    {
        InputManager::GetInstance().ClearBindings();

        if (m_pHeadlineObject)  { m_pHeadlineObject->MarkForDestroy();  m_pHeadlineObject  = nullptr; }
        if (m_pScoreObject)     { m_pScoreObject->MarkForDestroy();     m_pScoreObject     = nullptr; }
        if (m_pHintObject)      { m_pHintObject->MarkForDestroy();      m_pHintObject      = nullptr; }
        if (m_pLetter0Object)   { m_pLetter0Object->MarkForDestroy();   m_pLetter0Object   = nullptr; }
        if (m_pLetter1Object)   { m_pLetter1Object->MarkForDestroy();   m_pLetter1Object   = nullptr; }
        if (m_pLetter2Object)   { m_pLetter2Object->MarkForDestroy();   m_pLetter2Object   = nullptr; }
        if (m_pReadyObject)     { m_pReadyObject->MarkForDestroy();     m_pReadyObject     = nullptr; }
        if (m_pEntryNavObject)  { m_pEntryNavObject->MarkForDestroy();  m_pEntryNavObject  = nullptr; }
        m_pEntryNav = nullptr;
    }

    std::unique_ptr<GameState> HighScoreState::Update(GameManagerComponent* manager)
    {
        if (!manager->IsStartRequested())
            return nullptr;

        if (!m_AlreadySaved && m_pEntryNav)
        {
            m_AlreadySaved = true;
            HighScoreRepository::Save(
                ResourceManager::GetInstance().GetDataPath(),
                m_pEntryNav->GetInitials(),
                m_FinalScore);
        }

        return std::make_unique<ShowHighScoreState>();
    }
}