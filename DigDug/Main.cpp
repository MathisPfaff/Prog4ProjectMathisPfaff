#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "TextComponent.h"
#include "TextureComponent.h"
#include "Scene.h"
#include "FPSComponent.h"
#include "GameObject.h"
#include "InputManager.h"
#include "MoveCommand.h"
#include "HealthComponent.h"
#include "ScoreComponent.h"
#include "SteamAchievementObserver.h"
#include "ServiceLocator.h"
#include "SoundObserver.h"

#include <filesystem>
#include <glm/glm.hpp>
namespace fs = std::filesystem;

static void load()
{
	auto& scene = dae::SceneManager::GetInstance().CreateScene();

	// Load & start sounds
	auto& sounds = dae::ServiceLocator::GetSoundSystem();
	sounds.LoadMusic(DigDugSounds::InGameMusic, "Data/In-Game Music.mp3");
	sounds.PlayMusic(DigDugSounds::InGameMusic, 0.6f, true);

	auto go = std::make_unique<dae::GameObject>();
	go->AddComponent<dae::TextureComponent>("background.png");
	scene.Add(std::move(go));

	go = std::make_unique<dae::GameObject>();
	go->AddComponent<dae::TextureComponent>("logo.png");
	go->SetLocalPosition(358, 180);
	scene.Add(std::move(go));

	auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	go = std::make_unique<dae::GameObject>();
	go->SetLocalPosition(292, 20);
	go->AddComponent<dae::TextComponent>("Programming 4 Assignment", font, SDL_Color{ 255, 255, 0, 255 });
	scene.Add(std::move(go));

	// FPS Counter
	auto fpsFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 24);
	auto fpsCounter = std::make_unique<dae::GameObject>();
	fpsCounter->SetLocalPosition(10, 10);
	fpsCounter->AddComponent<dae::TextComponent>("FPS: 0.0", fpsFont, SDL_Color{ 0, 255, 0, 255 });
	fpsCounter->AddComponent<dae::FPSComponent>();
	scene.Add(std::move(fpsCounter));

	auto uiFont    = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 20);
	auto smallFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 16);

	// --- Controls display ---
	constexpr SDL_Color p1Color{ 255, 100, 100, 255 };
	constexpr SDL_Color p2Color{ 100, 150, 255, 255 };

	// --- Player 1 ---
	auto player1 = std::make_unique<dae::GameObject>();
	player1->AddComponent<dae::TextureComponent>("DigDugBasicEnemy.png");
	player1->SetLocalPosition(300.f, 300.f);
	auto* pPlayer1    = player1.get();
	scene.Add(std::move(player1));

	// --- Player 2 ---
	auto player2 = std::make_unique<dae::GameObject>();
	player2->AddComponent<dae::TextureComponent>("DigDugBasicEnemy.png");
	player2->SetLocalPosition(500.f, 300.f);
	auto* pPlayer2    = player2.get();
	scene.Add(std::move(player2));

#if USE_STEAMWORKS
	// Steam achievement observers — static so they outlive load() for the full session
	static dae::SteamAchievementObserver steamObs1(scoreComp1);
	static dae::SteamAchievementObserver steamObs2(scoreComp2);
#endif

	constexpr float moveSpeed = 150.f;
	auto& input = dae::InputManager::GetInstance();

	// Keyboard — Player 1 movement
	input.BindKeyboard(SDL_SCANCODE_W, dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{  0.f, -1.f }, moveSpeed));
	input.BindKeyboard(SDL_SCANCODE_S, dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{  0.f,  1.f }, moveSpeed));
	input.BindKeyboard(SDL_SCANCODE_A, dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{ -1.f,  0.f }, moveSpeed));
	input.BindKeyboard(SDL_SCANCODE_D, dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer1, glm::vec2{  1.f,  0.f }, moveSpeed));

	// Controller 0 DPad — Player 2 movement
	input.BindController(0, dae::Controller::ControllerButton::DPadUp,    dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{  0.f, -1.f }, moveSpeed * 2));
	input.BindController(0, dae::Controller::ControllerButton::DPadDown,  dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{  0.f,  1.f }, moveSpeed * 2));
	input.BindController(0, dae::Controller::ControllerButton::DPadLeft,  dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{ -1.f,  0.f }, moveSpeed * 2));
	input.BindController(0, dae::Controller::ControllerButton::DPadRight, dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{  1.f,  0.f }, moveSpeed * 2));

	auto ctrl1 = std::make_unique<dae::GameObject>();
	ctrl1->SetLocalPosition(200.f, 470.f);
	ctrl1->AddComponent<dae::TextComponent>("P1: WASD to move", smallFont, p1Color);
	scene.Add(std::move(ctrl1));

	auto ctrl2 = std::make_unique<dae::GameObject>();
	ctrl2->SetLocalPosition(200.f, 490.f);
	ctrl2->AddComponent<dae::TextComponent>("P2: D-Pad to move", smallFont, p2Color);
	scene.Add(std::move(ctrl2));
}

int main(int, char*[]) {
#if __EMSCRIPTEN__
	fs::path data_location = "";
#else
	fs::path data_location = "./Data/";
	if(!fs::exists(data_location))
		data_location = "../Data/";
#endif
	dae::Minigin engine(data_location);
	engine.Run(load);
	return 0;
}