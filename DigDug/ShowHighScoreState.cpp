#include "ShowHighScoreState.h"
#include "MainMenuState.h"
#include "GameManagerComponent.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "TextComponent.h"
#include "InputManager.h"
#include "EnterCommand.h"
#include "HighScoreRepository.h"
#include "Controller.h"
#include <string>
#include <SDL3/SDL.h>

namespace dae
{
    static constexpr float kSHSTitleX = 165.f;
    static constexpr float kSHSTitleY = 50.f;
    static constexpr float kSHSEntryX = 220.f;
    static constexpr float kSHSEntryY = 120.f;
    static constexpr float kSHSSpacing = 38.f;
    static constexpr float kSHSHintX = 185.f;
    static constexpr float kSHSHintY = 570.f;

    void ShowHighScoreState::OnEnter(GameManagerComponent* manager)
    {
        manager->ResetStartRequest();

        auto& scene = SceneManager::GetInstance().GetActiveScene();
        auto& res = ResourceManager::GetInstance();

        auto titleFont = res.LoadFont("Lingua.otf", 40);
        auto entryFont = res.LoadFont("Lingua.otf", 26);
        auto hintFont = res.LoadFont("Lingua.otf", 18);

        // ── Title ─────────────────────────────────────────────────────────────
        {
            auto obj = std::make_unique<GameObject>();
            obj->SetLocalPosition(kSHSTitleX, kSHSTitleY);
            obj->AddComponent<TextComponent>("HIGH SCORES", titleFont, SDL_Color{ 255, 215, 0, 255 });
            m_pTitleObject = obj.get();
            scene.Add(std::move(obj));
        }

        // ── Entries ───────────────────────────────────────────────────────────
        const auto entries = HighScoreRepository::Load(res.GetDataPath());

        for (int i = 0; i < static_cast<int>(entries.size()); ++i)
        {
            const auto& e = entries[i];

            // Format: "1.   AAA   12345"
            const std::string rank = std::to_string(i + 1) + ".";
            const std::string line = rank + "   " + e.name + "   " + std::to_string(e.score);

            auto obj = std::make_unique<GameObject>();
            obj->SetLocalPosition(kSHSEntryX, kSHSEntryY + i * kSHSSpacing);
            obj->AddComponent<TextComponent>(line, entryFont, SDL_Color{ 255, 255, 255, 255 });
            m_pEntryObjects.push_back(obj.get());
            scene.Add(std::move(obj));
        }

        // ── Hint ──────────────────────────────────────────────────────────────
        {
            auto obj = std::make_unique<GameObject>();
            obj->SetLocalPosition(kSHSHintX, kSHSHintY);
            obj->AddComponent<TextComponent>("Press Enter / Start to return to menu",
                hintFont, SDL_Color{ 180, 180, 180, 255 });
            m_pHintObject = obj.get();
            scene.Add(std::move(obj));
        }

        // ── Input ─────────────────────────────────────────────────────────────
        auto& input = InputManager::GetInstance();
        input.BindKeyboard(SDL_SCANCODE_RETURN, KeyState::Pressed,
            std::make_unique<EnterCommand>(manager));
        input.BindController(0, Controller::ControllerButton::Start, KeyState::Pressed,
            std::make_unique<EnterCommand>(manager));
        input.BindController(0, Controller::ControllerButton::ButtonA, KeyState::Pressed,
            std::make_unique<EnterCommand>(manager));
    }

    void ShowHighScoreState::OnExit(GameManagerComponent*)
    {
        InputManager::GetInstance().ClearBindings();

        if (m_pTitleObject) { m_pTitleObject->MarkForDestroy(); m_pTitleObject = nullptr; }
        if (m_pHintObject) { m_pHintObject->MarkForDestroy();  m_pHintObject = nullptr; }

        for (auto* obj : m_pEntryObjects)
            if (obj) obj->MarkForDestroy();
        m_pEntryObjects.clear();
    }

    std::unique_ptr<GameState> ShowHighScoreState::Update(GameManagerComponent* manager)
    {
        if (manager->IsStartRequested())
            return std::make_unique<MainMenuState>();

        return nullptr;
    }
}