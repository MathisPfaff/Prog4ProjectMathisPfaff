#pragma once
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "Transform.h"
#include "BaseComponent.h"

namespace dae
{
	class BaseComponent;

	class GameObject final
	{
	private:
		Transform m_transform{};
		glm::vec3 m_worldPosition{};
		bool m_PositionIsDirty{ true };

		std::vector<std::unique_ptr<BaseComponent>> m_pComponents;
		std::vector<std::unique_ptr<BaseComponent>> m_pDeleteComponents;

		GameObject* m_pParent = nullptr;
		std::vector<GameObject*> m_pChildren;

		bool m_MarkedForDestroy{};
	public:
		void FixedUpdate(float fixed_time_step);
		void Update();
		void LateUpdate();
		void Render() const;

		void SetLocalPosition(float x, float y);
		void SetLocalPosition(const glm::vec3& position);
		const glm::vec3& GetWorldPosition();

		GameObject() = default;
		~GameObject();
		GameObject(const GameObject& other) = delete;
		GameObject(GameObject&& other) = delete;
		GameObject& operator=(const GameObject& other) = delete;
		GameObject& operator=(GameObject&& other) = delete;

		void MarkForDestroy() { m_MarkedForDestroy = true; }
		bool IsMarkedForDestroy() const { return m_MarkedForDestroy; }

		GameObject* GetParent() const { return m_pParent; }
		void SetParent(GameObject* parent, bool keepWorldPosition);

		int GetChildCount() const { return static_cast<int>(m_pChildren.size()); }
		GameObject* GetChild(int index) const;
		void AddChild(GameObject* child);
		void RemoveChild(GameObject* child);
		bool IsChild(GameObject* child) const;

		void SetPositionDirty();

		template <typename T>
		T* GetComponent()
		{
			for (const auto& component : m_pComponents)
			{
				if (dynamic_cast<T*>(component.get()))
					return static_cast<T*>(component.get());
			}
			return nullptr;
		}

		template <typename T, typename... Args>
		T* AddComponent(Args&&... args)
		{
			static_assert(std::is_base_of<BaseComponent, T>::value, "T must derive from BaseComponent");
			auto newComponent = std::make_unique<T>(this, std::forward<Args>(args)...);
			T* rawPtr = newComponent.get();
			m_pComponents.push_back(std::move(newComponent));
			return rawPtr;
		}

		void RemoveComponent(BaseComponent* pComponent);
	};
}
