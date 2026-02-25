#pragma once
#include <glm/glm.hpp>
#include "BaseComponent.h"

namespace dae
{
	class GameObject;

	class RotationComponent final : public BaseComponent
	{
	public:
		virtual ~RotationComponent() = default;
		RotationComponent(const RotationComponent& other) = delete;
		RotationComponent(RotationComponent&& other) = delete;
		RotationComponent& operator=(const RotationComponent& other) = delete;
		RotationComponent& operator=(RotationComponent&& other) = delete;

		void FixedUpdate(float deltaTime) override;
		void Update() override;
		void LateUpdate() override;
		void Render() const override;

		RotationComponent(GameObject* owner, float rotationSpeed, GameObject* target);
		RotationComponent(GameObject* owner, float rotationSpeed, const glm::vec3& rotationCenter);

	private:
		float m_RotationSpeed{};
		GameObject* m_pTarget{ nullptr };
		glm::vec3 m_RotationCenter{};
	};
}