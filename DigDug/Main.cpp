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

#include <filesystem>
#include <glm/glm.hpp>
namespace fs = std::filesystem;

static constexpr float kSidebarWidth = 155.f;
static constexpr float kGridOffsetY  = 30.f;
static constexpr int   kGridCols     = 16;
static constexpr int   kGridRows     = 14;
static constexpr float kCellSize     = 36.f;
static constexpr float kGridWidth    = kGridCols * kCellSize;
static constexpr float kGridHeight   = kGridRows * kCellSize;

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

    auto ctrlHeader = std::make_unique<dae::GameObject>();
    ctrlHeader->SetLocalPosition(8.f, 40.f);
    ctrlHeader->AddComponent<dae::TextComponent>("Controls:", smallFont, SDL_Color{ 220, 220, 220, 255 });
    scene.Add(std::move(ctrlHeader));

    auto ctrl1 = std::make_unique<dae::GameObject>();
    ctrl1->SetLocalPosition(8.f, 60.f);
    ctrl1->AddComponent<dae::TextComponent>("P1: WASD", smallFont, p1Color);
    scene.Add(std::move(ctrl1));

    auto ctrl2 = std::make_unique<dae::GameObject>();
    ctrl2->SetLocalPosition(8.f, 78.f);
    ctrl2->AddComponent<dae::TextComponent>("P2: D-Pad", smallFont, p2Color);
    scene.Add(std::move(ctrl2));

    auto pGridObject = std::make_unique<dae::GameObject>();
    pGridObject->SetLocalPosition(kSidebarWidth, kGridOffsetY);
    pGridObject->AddComponent<dae::GridComponent>(kGridWidth, kGridHeight, kGridCols, kGridRows);
    auto* pGrid    = pGridObject->GetComponent<dae::GridComponent>();
    auto* pGridRaw = pGridObject.get();

    DigShapeA(pGrid);
    DigShapeB(pGrid);
    DigShapeC(pGrid);

    scene.Add(std::move(pGridObject));

    float p1X{}, p1Y{};
    pGrid->CellToWorld(1, 1, p1X, p1Y);
    auto player1 = std::make_unique<dae::GameObject>();
    player1->AddComponent<dae::TextureComponent>("Player.png", 1.5f);
    player1->SetLocalPosition(pGridRaw->GetWorldPosition() + glm::vec3(p1X, p1Y, 0.f));
    auto* pPlayer1 = player1.get();
    scene.Add(std::move(player1));

    float p2X{}, p2Y{};
    pGrid->CellToWorld(6, 5, p2X, p2Y);
    auto player2 = std::make_unique<dae::GameObject>();
    player2->AddComponent<dae::TextureComponent>("Player.png", 1.5f);
    player2->SetLocalPosition(pGridRaw->GetWorldPosition() + glm::vec3(p2X, p2Y, 0.f));
    auto* pPlayer2 = player2.get();
    scene.Add(std::move(player2));

#if USE_STEAMWORKS
    static dae::SteamAchievementObserver steamObs1(scoreComp1);
    static dae::SteamAchievementObserver steamObs2(scoreComp2);
#endif

    auto& input = dae::InputManager::GetInstance();

    input.BindKeyboard(SDL_SCANCODE_W, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{  0.f, -1.f }, pGridRaw));
    input.BindKeyboard(SDL_SCANCODE_S, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{  0.f,  1.f }, pGridRaw));
    input.BindKeyboard(SDL_SCANCODE_A, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{ -1.f,  0.f }, pGridRaw));
    input.BindKeyboard(SDL_SCANCODE_D, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{  1.f,  0.f }, pGridRaw));

    input.BindController(0, dae::Controller::ControllerButton::DPadUp,    dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{  0.f, -1.f }, pGridRaw));
    input.BindController(0, dae::Controller::ControllerButton::DPadDown,  dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{  0.f,  1.f }, pGridRaw));
    input.BindController(0, dae::Controller::ControllerButton::DPadLeft,  dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{ -1.f,  0.f }, pGridRaw));
    input.BindController(0, dae::Controller::ControllerButton::DPadRight, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{  1.f,  0.f }, pGridRaw));

    auto pooka = std::make_unique<dae::GameObject>();
    pooka->AddComponent<dae::TextureComponent>("Pooka.png", 1.5f);
    pooka->SetLocalPosition(pGridRaw->GetWorldPosition() + glm::vec3(200.f, 150.f, 0.f));
    pooka->AddComponent<dae::PookaComponent>();
    scene.Add(std::move(pooka));
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
    dae::Minigin engine(dataLocation);
    engine.Run(load);
    return 0;
}