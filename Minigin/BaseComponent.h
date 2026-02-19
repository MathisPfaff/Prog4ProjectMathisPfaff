#pragma once
#include "GameObject.h"

namespace dae
{
	class GameObject;
	class BaseComponent
	{
	private:
		GameObject* m_pOwner{};
	public:
		BaseComponent(GameObject* owner) : m_pOwner(owner) {}
		virtual ~BaseComponent();
		BaseComponent(const BaseComponent& other) = delete;
		BaseComponent(BaseComponent&& other) = delete;
		virtual BaseComponent& operator=(const BaseComponent& other) = delete;
		virtual BaseComponent& operator=(BaseComponent&& other) = delete;

		virtual void FixedUpdate(float deltaTime) = 0;
		virtual void Update() = 0;
		virtual void LateUpdate() = 0;
		virtual void Render() const = 0;

		GameObject* GetOwner() const;
	};
}