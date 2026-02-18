#pragma once
#include "BaseComponent.h"

namespace dae
{
	class TextObject;

	class FPSComponent final : public BaseComponent
	{
	public:
		FPSComponent(GameObject* owner);
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
		TextObject* m_pTextObject{ nullptr };

		void CalculateFPS(float delta_time);
	};
}
