#include "MoveCommand.h"
#include "GameObject.h"
#include "GameTime.h"

namespace dae
{
	MoveCommand::MoveCommand(GameObject* pGameObject, glm::vec2 direction, float speed)
		: GameObjectCommand(pGameObject)
		, m_Direction(direction)
		, m_Speed(speed)
	{
	}

	void MoveCommand::Execute()
	{
		auto* pGO = GetGameObject();
		if (!pGO) return;

		const float dt = GameTime::GetInstance().GetDeltaTime();
		const glm::vec3 currentPos = pGO->GetLocalPosition();
		pGO->SetLocalPosition(currentPos + glm::vec3(m_Direction.x, m_Direction.y, 0.f) * m_Speed * dt);
	}
}