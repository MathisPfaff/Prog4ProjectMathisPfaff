#pragma once
#include "Observer.h"
#include "HealthComponent.h"

namespace dae
{
    class TextComponent;
    class GameObject;

    class LivesDisplayObserver final : public Observer
    {
    public:
        LivesDisplayObserver(HealthComponent* healthComp, TextComponent* textComp);
        ~LivesDisplayObserver() override;

        void OnNotify(BaseComponent* entity, unsigned int eventID) override;

        // Called when the observed components are about to be destroyed
        void ClearReferences();

    private:
        HealthComponent* m_HealthComp{};
        TextComponent* m_TextComp{};

        void UpdateDisplay();
    };
}