#pragma once
#include <functional>
#include <string>
#include "BaseComponent.h"
#include "Observer.h"

namespace dae
{
	class TextDisplayComponent final : public BaseComponent, public Observer
	{
	public:
		// subscribeFunc lets the caller wire any subject (health, score, …) without coupling this class to them
		TextDisplayComponent(GameObject* owner,
			std::function<std::string()> textGetter,
			std::function<void(Observer*)> subscribeFunc = nullptr);

		virtual ~TextDisplayComponent() override = default;
		TextDisplayComponent(const TextDisplayComponent&) = delete;
		TextDisplayComponent(TextDisplayComponent&&) = delete;
		TextDisplayComponent& operator=(const TextDisplayComponent&) = delete;
		TextDisplayComponent& operator=(TextDisplayComponent&&) = delete;

		void FixedUpdate(float) override {}
		void Update() override;
		void LateUpdate() override {}
		void Render() const override {}

		void OnNotify(BaseComponent*, Event) override { m_Dirty = true; }

	private:
		std::function<std::string()> m_TextGetter;
		bool m_Dirty{ true }; // true initially so first frame renders
	};
}