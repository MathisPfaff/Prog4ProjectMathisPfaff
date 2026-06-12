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
#include "GridComponent.h"
#include "GameManagerComponent.h"

#include <filesystem>
#include <glm/glm.hpp>
namespace fs = std::filesystem;

static constexpr float kSidebarWidth  = 155.f;
static constexpr float kGridOffsetY   = 30.f;
static constexpr int   kGridCols      = 14;
static constexpr int   kGridRows      = 16;
static constexpr float kCellSize      = 36.f;
static constexpr float kGridWidth     = kGridCols * kCellSize;
static constexpr float kGridHeight    = kGridRows * kCellSize;
static constexpr float kRightPadding  = 16.f;
static constexpr float kBottomPadding = 16.f;
static constexpr int   kWindowWidth   = static_cast<int>(kSidebarWidth + kGridWidth + kRightPadding);
static constexpr int   kWindowHeight  = static_cast<int>(kGridOffsetY  + kGridHeight + kBottomPadding);

static void DigShapeA(dae::GridComponent* grid)
{
    grid->PreDigCell(1, 1, dae::TunnelSide::Right | dae::TunnelSide::Down);
    grid->PreDigCell(2, 1, dae::TunnelSide::Left  | dae::TunnelSide::Right);
    grid->PreDigCell(3, 1, dae::TunnelSide::Left  | dae::TunnelSide::Right);
    grid->PreDigCell(4, 1, dae::TunnelSide::Left);
    grid->PreDigCell(1, 2, dae::TunnelSide::Up | dae::TunnelSide::Down);
    grid->PreDigCell(1, 3, dae::TunnelSide::Up | dae::TunnelSide::Down);
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
    grid->PreDigCell(9,  6, dae::TunnelSide::Up | dae::TunnelSide::Down);
    grid->PreDigCell(9,  7, dae::TunnelSide::Up);
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

    // ── FPS debug counter (not part of game world, stays on high score screen) ──
    auto fpsFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 20);
    auto fpsObj  = std::make_unique<dae::GameObject>();
    fpsObj->SetLocalPosition(8.f, 8.f);
    fpsObj->AddComponent<dae::TextComponent>("FPS: 0.0", fpsFont, SDL_Color{ 0, 255, 0, 255 });
    fpsObj->AddComponent<dae::FPSComponent>();
    scene.Add(std::move(fpsObj));

    // ── Grid ───────────────────────────────────────────────────────────────────
    auto gridObj = std::make_unique<dae::GameObject>();
    gridObj->SetLocalPosition(kSidebarWidth, kGridOffsetY);
    gridObj->AddComponent<dae::GridComponent>(kGridWidth, kGridHeight, kGridCols, kGridRows);
    auto* pGrid   = gridObj->GetComponent<dae::GridComponent>();
    auto* pGridRaw = gridObj.get();

    DigShapeA(pGrid);
    DigShapeB(pGrid);
    DigShapeC(pGrid);

    scene.Add(std::move(gridObj));

    // ── GameManager owns everything else: player, enemies, HUD, observers ──────
    auto managerObj = std::make_unique<dae::GameObject>();
    managerObj->AddComponent<dae::GameManagerComponent>(pGridRaw);
    scene.Add(std::move(managerObj));
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