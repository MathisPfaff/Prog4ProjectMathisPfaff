#include "FPSComponent.h"
#include "GameTime.h"
#include "TextComponent.h"
#include "GameObject.h"
#include <format>

void dae::FPSComponent::FixedUpdate(float deltaTime)
{
	(void)deltaTime;
}

void dae::FPSComponent::Update()
{
	if (!m_pTextComponent && GetOwner())
	{
		m_pTextComponent = GetOwner()->GetComponent<TextComponent>();
	}

	float deltaTime = dae::GameTime::GetInstance().GetDeltaTime();
	CalculateFPS(deltaTime);
}

void dae::FPSComponent::LateUpdate()
{
	// Update the text component with the new FPS value
	if (m_pTextComponent)
	{
		m_pTextComponent->SetText(std::format("FPS: {:.1f}", m_FPS));
	}
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
	}
}