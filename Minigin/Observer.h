#pragma once
#include <vector>

namespace dae
{
	class BaseComponent;
	class GameActor;

	class Observer
	{
	public:
		enum class Event
		{
			PlayerDied,
			HealthChanged,
			AddScore
		};

		virtual ~Observer();
		virtual void OnNotify(BaseComponent* entity, Event event) = 0;

		void AddSubject(GameActor* subject);
		void OnSubjectRemoved(GameActor* subject);
		void RemoveSubject(GameActor* subject);

	private:
		std::vector<GameActor*> m_Subjects;
	};
}