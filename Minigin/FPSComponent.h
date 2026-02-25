#pragma once
#include "BaseComponent.h"

namespace dae
{
	class TextComponent;

	class FPSComponent final : public BaseComponent
	{
	public:
		FPSComponent(GameObject* owner) : BaseComponent(owner) {};
		virtual ~FPSComponent() override = default;
		FPSComponent(const FPSComponent& other) = delete;
		FPSComponent(FPSComponent&& other) = delete;
		FPSComponent& operator=(const FPSComponent& other) = delete;
		FPSComponent& operator=(FPSComponent&& other) = delete;

		virtual void FixedUpdate(float delta_time) override;
		virtual void Update() override;
		virtual void LateUpdate() override;
		virtual void Render() const override;

		float GetFPS() const { return m_FPS; }

	private:
		float m_FPS{};
		int m_FrameCount{};
		float m_ElapsedTime{};
		bool m_fpsChanged{ false };
		TextComponent* m_pTextComponent{ nullptr };

		void CalculateFPS(float delta_time);
	};
}
