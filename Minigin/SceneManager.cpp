#include "SceneManager.h"
#include "Scene.h"
#include <stdexcept>

namespace dae
{
	void SceneManager::FixedUpdate(float fixed_time_step)
	{
		for (auto& scene : m_scenes)
			scene->FixedUpdate(fixed_time_step);
	}

	void SceneManager::Update()
	{
		for (auto& scene : m_scenes)
			scene->Update();
	}

	void SceneManager::LateUpdate()
	{
		for (auto& scene : m_scenes)
			scene->LateUpdate();
	}

	void SceneManager::Render()
	{
		for (const auto& scene : m_scenes)
			scene->Render();
	}

	Scene& SceneManager::CreateScene()
	{
		m_scenes.emplace_back(new Scene());
		return *m_scenes.back();
	}

	Scene& SceneManager::GetActiveScene()
	{
		if (m_scenes.empty())
			throw std::runtime_error("No scene exists yet.");
		return *m_scenes.back();
	}
}
