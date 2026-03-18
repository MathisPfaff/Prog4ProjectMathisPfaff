#pragma once
#include "Command.h"

namespace dae
{
	class HealthComponent;

	class DamageCommand final : public Command
	{
	public:
		explicit DamageCommand(HealthComponent* healthComp, int damage = 1);
		void Execute() override;

	private:
		HealthComponent* m_HealthComponent{};
		int m_Damage{ 1 };
	};
}