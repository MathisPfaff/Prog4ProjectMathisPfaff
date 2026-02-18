#pragma once
#include "Singleton.h"

namespace dae
{
	class GameTime final : public Singleton<GameTime>
	{
	public:
		void SetDeltaTime(float deltaTime) { m_DeltaTime = deltaTime; }
		float GetDeltaTime() const { return m_DeltaTime; }
	private:
		float m_DeltaTime{};
	};
}