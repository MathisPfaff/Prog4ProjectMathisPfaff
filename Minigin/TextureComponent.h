#pragma once
#include <string>
#include <memory>
#include "BaseComponent.h"

namespace dae
{
	class Texture2D;

	class TextureComponent final : public BaseComponent
	{
	public:
		TextureComponent(GameObject* owner, const std::string& filename);
		virtual ~TextureComponent() = default;
		TextureComponent(const TextureComponent& other) = delete;
		TextureComponent(TextureComponent&& other) = delete;
		TextureComponent& operator=(const TextureComponent& other) = delete;
		TextureComponent& operator=(TextureComponent&& other) = delete;

		void FixedUpdate(float delta_time) override;
		void Update() override;
		void LateUpdate() override;
		void Render() const override;

		void SetTexture(const std::string& filename);

	private:
		bool m_isDirty{ true };
		std::string m_filename{};
		std::shared_ptr<Texture2D> m_texture{};
	};
}