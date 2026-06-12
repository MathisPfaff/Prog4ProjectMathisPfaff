#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "TextComponent.h"
#include "Scene.h"
#include "FPSComponent.h"
#include "GameObject.h"
#include "InputManager.h"
#include "MoveCommand.h"
#include "GridComponent.h"
#include "TextureComponent.h"
#include "SteamAchievementObserver.h"
#include "ServiceLocator.h"
#include "SoundObserver.h"
#include "PookaComponent.h"
#include "PlayerMovementComponent.h"
#include "HealthComponent.h"
#include "HitboxComponent.h"
#include "LivesDisplayObserver.h"
#include "PumpComponent.h"
#include "PumpCommand.h"
#include "PumpHeldCommand.h"
#include "FireBreathComponent.h"
#include "FygarComponent.h"

#include <filesystem>
#include <glm/glm.hpp>
namespace fs = std::filesystem;

static constexpr float kSidebarWidth = 155.f;
static constexpr float kGridOffsetY  = 30.f;
static constexpr int   kGridCols     = 14;
static constexpr int   kGridRows     = 16;
static constexpr float kCellSize     = 36.f;
static constexpr float kGridWidth    = kGridCols * kCellSize;
static constexpr float kGridHeight   = kGridRows * kCellSize;
static constexpr float kRightPadding = 16.f;
static constexpr float kBottomPadding = 16.f;
static constexpr int   kWindowWidth = static_cast<int>(kSidebarWidth + kGridWidth + kRightPadding);
static constexpr int   kWindowHeight = static_cast<int>(kGridOffsetY + kGridHeight + kBottomPadding);

static void DigShapeA(dae::GridComponent* grid)
{
    grid->PreDigCell(1, 1, dae::TunnelSide::Right | dae::TunnelSide::Down);
    grid->PreDigCell(2, 1, dae::TunnelSide::Left  | dae::TunnelSide::Right);
    grid->PreDigCell(3, 1, dae::TunnelSide::Left  | dae::TunnelSide::Right);
    grid->PreDigCell(4, 1, dae::TunnelSide::Left);
    grid->PreDigCell(1, 2, dae::TunnelSide::Up   | dae::TunnelSide::Down);
    grid->PreDigCell(1, 3, dae::TunnelSide::Up   | dae::TunnelSide::Down);
    grid->PreDigCell(1, 4, dae::TunnelSide::Up);
}

static void DigShapeB(dae::GridComponent* grid)
{
    grid->PreDigCell(6,  5, dae::TunnelSide::Right);
    grid->PreDigCell(7,  5, dae::TunnelSide::Left | dae::TunnelSide::Right);
    grid->PreDigCell(8,  5, dae::TunnelSide::Left | dae::TunnelSide::Right);
    grid->PreDigCell(9,  5, dae::TunnelSide::Left | dae::TunnelSide::Right | dae::TunnelSide::Down);
    grid->PreDigCell(10, 5, dae::TunnelSide::Left | dae::TunnelSide::Right);
    grid->PreDigCell(11, 5, dae::TunnelSide::Left);
    grid->PreDigCell(9, 6, dae::TunnelSide::Up | dae::TunnelSide::Down);
    grid->PreDigCell(9, 7, dae::TunnelSide::Up);
}

static void DigShapeC(dae::GridComponent* grid)
{
    constexpr int colStart = 8, colEnd = 12;
    constexpr int rowStart = 9, rowEnd = 12;

    for (int col = colStart; col <= colEnd; ++col)
    {
        dae::TunnelSide topSides = dae::TunnelSide::None;
        if (col > colStart) topSides |= dae::TunnelSide::Left;
        if (col < colEnd)   topSides |= dae::TunnelSide::Right;
        if (col == colStart || col == colEnd) topSides |= dae::TunnelSide::Down;
        grid->PreDigCell(col, rowStart, topSides);

        dae::TunnelSide bottomSides = dae::TunnelSide::None;
        if (col > colStart) bottomSides |= dae::TunnelSide::Left;
        if (col < colEnd)   bottomSides |= dae::TunnelSide::Right;
        if (col == colStart || col == colEnd) bottomSides |= dae::TunnelSide::Up;
        grid->PreDigCell(col, rowEnd, bottomSides);
    }

    for (int row = rowStart + 1; row < rowEnd; ++row)
    {
        grid->PreDigCell(colStart, row, dae::TunnelSide::Up | dae::TunnelSide::Down);
        grid->PreDigCell(colEnd,   row, dae::TunnelSide::Up | dae::TunnelSide::Down);
    }
}

static void load()
{
    auto& scene = dae::SceneManager::GetInstance().CreateScene();

    auto fpsFont  = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 20);
    auto smallFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 14);

    constexpr SDL_Color p1Color{ 255, 100, 100, 255 };
    constexpr SDL_Color p2Color{ 100, 150, 255, 255 };

    auto fpsCounter = std::make_unique<dae::GameObject>();
    fpsCounter->SetLocalPosition(8.f, 8.f);
    fpsCounter->AddComponent<dae::TextComponent>("FPS: 0.0", fpsFont, SDL_Color{ 0, 255, 0, 255 });
    fpsCounter->AddComponent<dae::FPSComponent>();
    scene.Add(std::move(fpsCounter));

    auto pGridObject = std::make_unique<dae::GameObject>();
    pGridObject->SetLocalPosition(kSidebarWidth, kGridOffsetY);
    pGridObject->AddComponent<dae::GridComponent>(kGridWidth, kGridHeight, kGridCols, kGridRows);
    auto* pGrid    = pGridObject->GetComponent<dae::GridComponent>();
    auto* pGridRaw = pGridObject.get();

    DigShapeA(pGrid);
    DigShapeB(pGrid);
    DigShapeC(pGrid);

    scene.Add(std::move(pGridObject));

    // Player 1 with health and hitbox
    float p1X{}, p1Y{};
    pGrid->CellToWorld(1, 1, p1X, p1Y);
    auto player1 = std::make_unique<dae::GameObject>();
    player1->AddComponent<dae::TextureComponent>("Player.png", 2.f);
    player1->SetLocalPosition(pGridRaw->GetWorldPosition() + glm::vec3(p1X, p1Y, 0.f));

    // Add health (3 lives)
    auto* healthComp1 = player1->AddComponent<dae::HealthComponent>(3);

    // Add hitbox (36x36 for 3x3 subcells)
    player1->AddComponent<dae::HitboxComponent>(36.f, 36.f, dae::HitboxType::Player);

    auto* pPlayer1 = player1.get();
    player1->AddComponent<dae::PlayerMovementComponent>(pGridRaw);
    player1->AddComponent<dae::PumpComponent>(pGridRaw);
    scene.Add(std::move(player1));

    // Lives display for Player 1 - moved lower
    auto livesDisplay1 = std::make_unique<dae::GameObject>();
    livesDisplay1->SetLocalPosition(10.f, 50.f);  // Changed from 10.f to 50.f
    auto* livesText1 = livesDisplay1->AddComponent<dae::TextComponent>("Lives: 3", smallFont, SDL_Color{255, 255, 255, 255});
    static dae::LivesDisplayObserver livesObs1(healthComp1, livesText1);
    scene.Add(std::move(livesDisplay1));

    // Player 2 with health and hitbox
    float p2X{}, p2Y{};
    pGrid->CellToWorld(6, 5, p2X, p2Y);
    auto player2 = std::make_unique<dae::GameObject>();
    player2->AddComponent<dae::TextureComponent>("Player.png", 2.f);
    player2->SetLocalPosition(pGridRaw->GetWorldPosition() + glm::vec3(p2X, p2Y, 0.f));

    // Add health (3 lives)
    auto* healthComp2 = player2->AddComponent<dae::HealthComponent>(3);

    // Add hitbox
    player2->AddComponent<dae::HitboxComponent>(36.f, 36.f, dae::HitboxType::Player);

    auto* pPlayer2 = player2.get();
    player2->AddComponent<dae::PlayerMovementComponent>(pGridRaw);
    player2->AddComponent<dae::PumpComponent>(pGridRaw);
    scene.Add(std::move(player2));

    // Lives display for Player 2 - moved lower
    auto livesDisplay2 = std::make_unique<dae::GameObject>();
    livesDisplay2->SetLocalPosition(10.f, 70.f);  // Changed from 30.f to 70.f
    auto* livesText2 = livesDisplay2->AddComponent<dae::TextComponent>("Lives: 3", smallFont, SDL_Color{255, 255, 0, 255});
    static dae::LivesDisplayObserver livesObs2(healthComp2, livesText2);
    scene.Add(std::move(livesDisplay2));

#if USE_STEAMWORKS
    static dae::SteamAchievementObserver steamObs1(scoreComp1);
    static dae::SteamAchievementObserver steamObs2(scoreComp2);
#endif

    auto& input = dae::InputManager::GetInstance();

    input.BindKeyboard(SDL_SCANCODE_W, dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{  0.f, -1.f }));
    input.BindKeyboard(SDL_SCANCODE_S, dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{  0.f,  1.f }));
    input.BindKeyboard(SDL_SCANCODE_A, dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{ -1.f,  0.f }));
    input.BindKeyboard(SDL_SCANCODE_D, dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{  1.f,  0.f }));

    input.BindController(0, dae::Controller::ControllerButton::DPadUp,    dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{  0.f, -1.f }));
    input.BindController(0, dae::Controller::ControllerButton::DPadDown,  dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{  0.f,  1.f }));
    input.BindController(0, dae::Controller::ControllerButton::DPadLeft,  dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{ -1.f,  0.f }));
    input.BindController(0, dae::Controller::ControllerButton::DPadRight, dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{  1.f,  0.f }));

    // Pump – Pressed: start beam / +1.5f pulse per tap
    input.BindKeyboard(SDL_SCANCODE_SPACE, dae::KeyState::Pressed,
        std::make_unique<dae::PumpCommand>(pPlayer1));
    input.BindController(0, dae::Controller::ControllerButton::ButtonX, dae::KeyState::Pressed,
        std::make_unique<dae::PumpCommand>(pPlayer2));

    // Pump – Held: continuous inflation (+1.5 units/s while button is down)
    input.BindKeyboard(SDL_SCANCODE_SPACE, dae::KeyState::Held,
        std::make_unique<dae::PumpHeldCommand>(pPlayer1));
    input.BindController(0, dae::Controller::ControllerButton::ButtonX, dae::KeyState::Held,
        std::make_unique<dae::PumpHeldCommand>(pPlayer2));

    // Pooka enemy with hitbox
    float pookaX{}, pookaY{};
    pGrid->CellToWorld(3, 1, pookaX, pookaY);
    auto pooka = std::make_unique<dae::GameObject>();
    pooka->AddComponent<dae::TextureComponent>("Pooka.png", 2.f);
    pooka->SetLocalPosition(pGridRaw->GetWorldPosition() + glm::vec3(pookaX, pookaY, 0.f));
    pooka->AddComponent<dae::PookaComponent>(pGridRaw);

    // Add hitbox to Pooka (36x36 for 3x3 subcells, same as player)
    pooka->AddComponent<dae::HitboxComponent>(36.f, 36.f, dae::HitboxType::Enemy);

    scene.Add(std::move(pooka));

    // ── Fygar ─────────────────────────────────────────────────────────────
    float fygarX{}, fygarY{};
    pGrid->CellToWorld(9, 5, fygarX, fygarY);
    auto fygar = std::make_unique<dae::GameObject>();
    fygar->AddComponent<dae::TextureComponent>("Fygar.png", 2.f);
    fygar->SetLocalPosition(pGridRaw->GetWorldPosition() + glm::vec3(fygarX, fygarY, 0.f));
    fygar->AddComponent<dae::HitboxComponent>(36.f, 36.f, dae::HitboxType::Enemy);
    fygar->AddComponent<dae::FireBreathComponent>(pGridRaw);  // must be added before FygarComponent
    fygar->AddComponent<dae::FygarComponent>(pGridRaw);       // AI is baked in, like PookaComponent
    scene.Add(std::move(fygar));
}

int main(int, char*[])
{
#if __EMSCRIPTEN__
    fs::path dataLocation = "";
#else
    fs::path dataLocation = "./Data/";
    if (!fs::exists(dataLocation))
        dataLocation = "../Data/";
#endif
    dae::Minigin engine(dataLocation, kWindowWidth, kWindowHeight);
    engine.Run(load);
    return 0;
}