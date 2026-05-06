#include "TextureComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Texture2D.h"
#include "GameObject.h"

namespace dae
{
	TextureComponent::TextureComponent(GameObject* owner, const std::string& filename, float scale)
		: BaseComponent(owner), m_isDirty(true), m_filename(filename), m_scale(scale)
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

			float texW{}, texH{};
			SDL_GetTextureSize(m_texture->GetSDLTexture(), &texW, &texH);

			const float scaledW = texW * m_scale;
			const float scaledH = texH * m_scale;

			// Center the texture around the GameObject's position
			const float drawX = pos.x - scaledW * 0.5f;
			const float drawY = pos.y - scaledH * 0.5f;

			Renderer::GetInstance().RenderTexture(*m_texture, drawX, drawY, scaledW, scaledH);
		}
	}

	void TextureComponent::SetTexture(const std::string& filename)
	{
		m_filename = filename;
		m_isDirty = true;
	}
}