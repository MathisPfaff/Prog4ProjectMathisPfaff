#include "TextureComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Texture2D.h"
#include "GameObject.h"

namespace dae
{
	TextureComponent::TextureComponent(GameObject* owner, const std::string& filename)
		: BaseComponent(owner), m_isDirty(true), m_filename(filename)
	{ }

	void TextureComponent::FixedUpdate(float delta_time)
	{
		(void)delta_time;
	}

	void TextureComponent::Update()
	{
		if (m_isDirty)
		{
			m_texture = ResourceManager::GetInstance().LoadTexture(m_filename);
			m_isDirty = false;
		}
	}

	void TextureComponent::LateUpdate()
	{

	}

	void TextureComponent::Render() const
	{
		if (m_texture && GetOwner())
		{
			const auto& pos = GetOwner()->GetWorldPosition();
			Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
		}
	}

	void TextureComponent::SetTexture(const std::string& filename)
	{
		m_filename = filename;
		m_isDirty = true;
	}
}