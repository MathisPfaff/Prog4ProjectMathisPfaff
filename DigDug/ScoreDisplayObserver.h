#pragma once
#include "Observer.h"
#include "ScoreComponent.h"

namespace dae
{
    class TextComponent;

    class ScoreDisplayObserver final : public Observer
    {
    public:
        ScoreDisplayObserver(ScoreComponent* scoreComp, TextComponent* textComp);
        ~ScoreDisplayObserver() override = default;

        void OnNotify(BaseComponent* entity, unsigned int eventID) override;

    private:
        ScoreComponent* m_ScoreComp{};
        TextComponent* m_TextComp{};

        void UpdateDisplay();
    };
}