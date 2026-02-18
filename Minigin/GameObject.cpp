#include <string>
#include "GameObject.h"
#include "ResourceManager.h"
#include "Renderer.h"

dae::GameObject::~GameObject() 
{
    m_pComponents.clear();
	m_pDeleteComponents.clear();
}

void dae::GameObject::FixedUpdate(float fixed_time_step)
{
    for (const auto& component : m_pComponents)
    {
        component->FixedUpdate(fixed_time_step);
    }
}

void dae::GameObject::Update()
{
    for (const auto& component : m_pComponents)
    {
        component->Update();
    }
}

void dae::GameObject::LateUpdate()
{
    for (const auto& deleted : m_pDeleteComponents)
    {
        auto it = std::remove_if(m_pComponents.begin(), m_pComponents.end(),
            [&](const std::unique_ptr<BaseComponent>& comp) {
                return comp.get() == deleted.get();
            });
        m_pComponents.erase(it, m_pComponents.end());
    }
    m_pDeleteComponents.clear();

    for (const auto& component : m_pComponents)
        component->LateUpdate();
}

void dae::GameObject::Render() const
{
	if (m_texture)
	{
		const auto& pos = m_transform.GetPosition();
		Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
	}

    for (const auto& component : m_pComponents)
    {
        component->Render();
    }
}

void dae::GameObject::SetTexture(const std::string& filename)
{
	m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

void dae::GameObject::SetPosition(float x, float y)
{
	m_transform.SetPosition(x, y, 0.0f);
}

void dae::GameObject::RemoveComponent(BaseComponent* pComponent)
{
    auto it = std::find_if(m_pComponents.begin(), m_pComponents.end(),
        [pComponent](const std::unique_ptr<BaseComponent>& comp) {
            return comp.get() == pComponent;
        });
    if (it != m_pComponents.end())
        m_pDeleteComponents.push_back(std::move(*it));
}