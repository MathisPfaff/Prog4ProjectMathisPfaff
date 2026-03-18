#pragma once
#include <vector>
#include "Observer.h"

namespace dae
{
	class BaseComponent;

	class GameActor
	{
	public:
		explicit GameActor(BaseComponent* owner);
		~GameActor();
		GameActor(const GameActor&) = delete;
		GameActor(GameActor&&) = delete;
		GameActor& operator=(const GameActor&) = delete;
		GameActor& operator=(GameActor&&) = delete;

		void AddObserver(Observer* observer);
		void RemoveObserver(Observer* observer);
		void Notify(Observer::Event event);

	private:
		BaseComponent* m_Owner{};
		std::vector<Observer*> m_Observers;
	};
}