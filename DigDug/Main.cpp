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
#include "TextDisplayComponent.h"
#include "DamageCommand.h"
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
	sounds.LoadSound(DigDugSounds::HighScore, "Data/High Score.mp3");
	sounds.LoadSound(DigDugSounds::Miss, "Data/Miss.mp3");
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
	auto* healthComp1 = player1->AddComponent<dae::HealthComponent>(3);
	auto* scoreComp1  = player1->AddComponent<dae::ScoreComponent>();
	auto* pPlayer1    = player1.get();
	scene.Add(std::move(player1));

	// Lives HUD observes healthComp1 for HealthChanged / PlayerDied
	auto hud1 = std::make_unique<dae::GameObject>();
	hud1->SetLocalPosition(10.f, 40.f);
	hud1->AddComponent<dae::TextComponent>("", uiFont, p1Color);
	hud1->AddComponent<dae::TextDisplayComponent>(
		[healthComp1]() { return "P1 Lives: " + std::to_string(healthComp1->GetHealth()); },
		[healthComp1](dae::Observer* obs) { healthComp1->AddObserver(obs); });
	scene.Add(std::move(hud1));

	// Score HUD observes scoreComp1 for AddScore
	auto scoreHud1 = std::make_unique<dae::GameObject>();
	scoreHud1->SetLocalPosition(10.f, 65.f);
	scoreHud1->AddComponent<dae::TextComponent>("", uiFont, p1Color);
	scoreHud1->AddComponent<dae::TextDisplayComponent>(
		[scoreComp1]() { return "P1 Score: " + std::to_string(scoreComp1->GetScore()); },
		[scoreComp1](dae::Observer* obs) { scoreComp1->AddObserver(obs); });
	scene.Add(std::move(scoreHud1));

	// --- Player 2 ---
	auto player2 = std::make_unique<dae::GameObject>();
	player2->AddComponent<dae::TextureComponent>("DigDugBasicEnemy.png");
	player2->SetLocalPosition(500.f, 300.f);
	auto* healthComp2 = player2->AddComponent<dae::HealthComponent>(3);
	auto* scoreComp2  = player2->AddComponent<dae::ScoreComponent>();
	auto* pPlayer2    = player2.get();
	scene.Add(std::move(player2));

	// Lives HUD observes healthComp2
	auto hud2 = std::make_unique<dae::GameObject>();
	hud2->SetLocalPosition(10.f, 90.f);
	hud2->AddComponent<dae::TextComponent>("", uiFont, p2Color);
	hud2->AddComponent<dae::TextDisplayComponent>(
		[healthComp2]() { return "P2 Lives: " + std::to_string(healthComp2->GetHealth()); },
		[healthComp2](dae::Observer* obs) { healthComp2->AddObserver(obs); });
	scene.Add(std::move(hud2));

	// Score HUD observes scoreComp2
	auto scoreHud2 = std::make_unique<dae::GameObject>();
	scoreHud2->SetLocalPosition(10.f, 115.f);
	scoreHud2->AddComponent<dae::TextComponent>("", uiFont, p2Color);
	scoreHud2->AddComponent<dae::TextDisplayComponent>(
		[scoreComp2]() { return "P2 Score: " + std::to_string(scoreComp2->GetScore()); },
		[scoreComp2](dae::Observer* obs) { scoreComp2->AddObserver(obs); });
	scene.Add(std::move(scoreHud2));

	// Cross-player: scoreComp1 observes healthComp2 (P2 dies → P1 gets 500)
	//               scoreComp2 observes healthComp1 (P1 dies → P2 gets 500)
	healthComp1->AddObserver(scoreComp2);
	healthComp2->AddObserver(scoreComp1);

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

	// Keyboard B — Player 1 takes damage
	input.BindKeyboard(SDL_SCANCODE_B, dae::KeyState::Pressed, std::make_unique<dae::DamageCommand>(healthComp1));

	// Controller 0 DPad — Player 2 movement
	input.BindController(0, dae::Controller::ControllerButton::DPadUp,    dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{  0.f, -1.f }, moveSpeed * 2));
	input.BindController(0, dae::Controller::ControllerButton::DPadDown,  dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{  0.f,  1.f }, moveSpeed * 2));
	input.BindController(0, dae::Controller::ControllerButton::DPadLeft,  dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{ -1.f,  0.f }, moveSpeed * 2));
	input.BindController(0, dae::Controller::ControllerButton::DPadRight, dae::KeyState::Held, std::make_unique<dae::MoveCommand>(pPlayer2, glm::vec2{  1.f,  0.f }, moveSpeed * 2));

	// Controller 0 Button A — Player 2 takes damage
	input.BindController(0, dae::Controller::ControllerButton::ButtonA, dae::KeyState::Pressed,
		std::make_unique<dae::DamageCommand>(healthComp2));

	auto ctrl1 = std::make_unique<dae::GameObject>();
	ctrl1->SetLocalPosition(200.f, 470.f);
	ctrl1->AddComponent<dae::TextComponent>("P1: WASD to move  |  B to take damage", smallFont, p1Color);
	scene.Add(std::move(ctrl1));

	auto ctrl2 = std::make_unique<dae::GameObject>();
	ctrl2->SetLocalPosition(200.f, 490.f);
	ctrl2->AddComponent<dae::TextComponent>("P2: D-Pad to move  |  Button A to take damage", smallFont, p2Color);
	scene.Add(std::move(ctrl2));

	// Sound observer
	static dae::SoundObserver soundObs{};
	healthComp1->AddObserver(&soundObs);
	healthComp2->AddObserver(&soundObs);
	scoreComp1->AddObserver(&soundObs);
	scoreComp2->AddObserver(&soundObs);
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