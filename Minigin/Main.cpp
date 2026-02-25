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
#include "RotationComponent.h"

#include <filesystem>
#include <glm/glm.hpp>
namespace fs = std::filesystem;

static void load()
{
	auto& scene = dae::SceneManager::GetInstance().CreateScene();

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

	auto pivot = std::make_unique<dae::GameObject>();
	pivot->SetLocalPosition(400.f, 388.f);
	pivot->AddComponent<dae::TextureComponent>("DigDugBasicEnemy.png");
	pivot->AddComponent<dae::RotationComponent>(180.f, glm::vec3{ 440.f, 388.f, 0.f });

	auto orbiter = std::make_unique<dae::GameObject>();
	orbiter->SetParent(pivot.get(), true);
	orbiter->SetLocalPosition(-50.f, 0.f);
	orbiter->AddComponent<dae::TextureComponent>("DigDugBasicEnemy.png");
	orbiter->AddComponent<dae::RotationComponent>(-270.f, orbiter->GetParent());

	scene.Add(std::move(pivot));
	scene.Add(std::move(orbiter));
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
