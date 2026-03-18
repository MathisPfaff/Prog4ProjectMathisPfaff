#pragma once
#include <string>
#include "BaseComponent.h"
#include "Observer.h"

namespace dae
{
	class HealthComponent;

	class HealthDisplayComponent final : public BaseComponent, public Observer
	{
	public:
		HealthDisplayComponent(GameObject* owner, HealthComponent* healthComp, const std::string& label = "Lives: ");
		virtual ~HealthDisplayComponent() override = default;
		HealthDisplayComponent(const HealthDisplayComponent&) = delete;
		HealthDisplayComponent(HealthDisplayComponent&&) = delete;
		HealthDisplayComponent& operator=(const HealthDisplayComponent&) = delete;
		HealthDisplayComponent& operator=(HealthDisplayComponent&&) = delete;

		void FixedUpdate(float) override {}
		void Update() override;
		void LateUpdate() override {}
		void Render() const override {}

		void OnNotify(BaseComponent* entity, Event event) override;

	private:
		HealthComponent* m_HealthComponent{};
		std::string m_Label{};
		bool m_isDirty{ true }; // true on first frame to set initial text
	};
}