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
#include "GameManagerComponent.h"
#include "ServiceLocator.h"
#include "SoundIds.h"

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

static void load()
{
    // ── Load sounds ───────────────────────────────────────────────────────────
    const std::string data = dae::ResourceManager::GetInstance().GetDataPath().string();
    auto& ss = dae::ServiceLocator::GetSoundSystem();

    ss.LoadSound(DigDugSounds::Shot,    data + "Dig Dug - Shot.mp3");
    ss.LoadSound(DigDugSounds::Pumping, data + "Dig Dug - Pumping.mp3");
    // Add your other sound files below when you have them:
    // ss.LoadSound(DigDugSounds::Miss,       data + "Dig Dug - Miss.mp3");
    // ss.LoadSound(DigDugSounds::HighScore,  data + "Dig Dug - HighScore.mp3");
    // ss.LoadMusic(DigDugSounds::InGameMusic, data + "Dig Dug - InGameMusic.mp3");

    // ── FPS counter ───────────────────────────────────────────────────────────
    auto& scene = dae::SceneManager::GetInstance().CreateScene();

    auto fpsFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 20);
    auto fpsObj  = std::make_unique<dae::GameObject>();
    fpsObj->SetLocalPosition(8.f, 8.f);
    fpsObj->AddComponent<dae::TextComponent>("FPS: 0.0", fpsFont, SDL_Color{ 0, 255, 0, 255 });
    fpsObj->AddComponent<dae::FPSComponent>();
    scene.Add(std::move(fpsObj));

    // ── GameManager ───────────────────────────────────────────────────────────
    auto managerObj = std::make_unique<dae::GameObject>();
    managerObj->AddComponent<dae::GameManagerComponent>();
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