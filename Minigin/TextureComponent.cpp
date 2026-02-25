#include "TextureComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Texture2D.h"
#include "GameObject.h"

dae::TextureComponent::TextureComponent(GameObject* owner, const std::string& filename)
	: BaseComponent(owner), m_isDirty(true), m_filename(filename)
{ }

void dae::TextureComponent::FixedUpdate(float delta_time)
{
	(void)delta_time;
}

void dae::TextureComponent::Update()
{
	if (m_isDirty)
	{
		m_texture = ResourceManager::GetInstance().LoadTexture(m_filename);
		m_isDirty = false;
	}
}

void dae::TextureComponent::LateUpdate()
{

}

void dae::TextureComponent::Render() const
{
	if (m_texture && GetOwner())
	{
		const auto& pos = GetOwner()->GetTransform().GetPosition();
		Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
	}
}

void dae::TextureComponent::SetTexture(const std::string& filename)
{
	m_filename = filename;
	m_isDirty = true;
}