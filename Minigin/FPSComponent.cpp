#include "FPSComponent.h"
#include "GameTime.h"
#include "TextObject.h"
#include <format>

dae::FPSComponent::FPSComponent(GameObject* owner) :
	dae::BaseComponent(owner) 
{
	// Try to get TextObject from owner
	m_pTextObject = dynamic_cast<TextObject*>(owner);
}

void dae::FPSComponent::FixedUpdate(float deltaTime)
{
	(void)deltaTime;
}

void dae::FPSComponent::Update()
{
	float deltaTime = dae::GameTime::GetInstance().GetDeltaTime();
	CalculateFPS(deltaTime);
}

void dae::FPSComponent::LateUpdate()
{
}

void dae::FPSComponent::Render() const
{
}

void dae::FPSComponent::CalculateFPS(float deltaTime)
{
	m_ElapsedTime += deltaTime;
	++m_FrameCount;

	if (m_ElapsedTime >= 0.3f)
	{
		m_FPS = static_cast<float>(m_FrameCount) / m_ElapsedTime;
		m_FrameCount = 0;
		m_ElapsedTime = 0.0f;

		// Update the text object with the new FPS value
		if (m_pTextObject)
		{
			m_pTextObject->SetText(std::format("FPS: {:.1f}", m_FPS));
		}
	}
}