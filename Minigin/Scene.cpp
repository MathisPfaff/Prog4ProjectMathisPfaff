#include <algorithm>
#include "Scene.h"

using namespace dae;

void Scene::Add(std::unique_ptr<GameObject> object)
{
	assert(object != nullptr && "Cannot add a null GameObject to the scene.");
	m_objects.emplace_back(std::move(object));
}

void Scene::Remove(GameObject& object)
{
	object.MarkForDestroy();
}

void Scene::RemoveAll()
{
	m_objects.clear();
}

void Scene::FixedUpdate(float fixed_time_step)
{
	for (auto& object : m_objects)
	{
		object->FixedUpdate(fixed_time_step);
	}
}

void Scene::Update()
{
	for(auto& object : m_objects)
	{
		object->Update();
	}
}

void Scene::LateUpdate()
{
	for (auto& object : m_objects)
	{
		object->LateUpdate();
	}

	auto it = std::remove_if(m_objects.begin(), m_objects.end(),
		[this](std::unique_ptr<GameObject>& obj)
		{
			if (obj->IsMarkedForDestroy())
			{
				m_deleteObjects.emplace_back(std::move(obj));
				return true;
			}
			return false;
		});
	m_objects.erase(it, m_objects.end());

	m_deleteObjects.clear();
}

void Scene::Render() const
{
	for (const auto& object : m_objects)
	{
		object->Render();
	}
}

