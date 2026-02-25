#pragma once
#include <memory>
#include <vector>
#include "Transform.h"
#include "BaseComponent.h"

namespace dae
{
	class BaseComponent;

	class GameObject final
	{
	private:
		Transform m_transform{};

		std::vector<std::unique_ptr<BaseComponent>> m_pComponents;
		std::vector<std::unique_ptr<BaseComponent>> m_pDeleteComponents;

		bool m_MarkedForDestroy{};
	public:
		void FixedUpdate(float fixed_time_step);
		void Update();
		void LateUpdate();
		void Render() const;

		void SetPosition(float x, float y);
		Transform& GetTransform() { return m_transform; }
		const Transform& GetTransform() const { return m_transform; }

		GameObject() = default;
		~GameObject();
		GameObject(const GameObject& other) = delete;
		GameObject(GameObject&& other) = delete;
		GameObject& operator=(const GameObject& other) = delete;
		GameObject& operator=(GameObject&& other) = delete;

		void MarkForDestroy() { m_MarkedForDestroy = true; }
		bool IsMarkedForDestroy() const { return m_MarkedForDestroy; }

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
