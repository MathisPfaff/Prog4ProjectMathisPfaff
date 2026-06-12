#include "PookaInflatingState.h"
#include "PookaWalkingState.h"
#include "GameObject.h"
#include "TextureComponent.h"
#include "HitboxComponent.h"
#include <memory>

namespace dae
{
    PookaInflatingState::PookaInflatingState(GameObject* pGridObject)
        : m_pGridObject(pGridObject)
    {}

    void PookaInflatingState::OnEnter(GameObject* owner)
    {
        // TODO: swap for a dedicated inflate sprite ("PookaInflate.png")
        if (auto* tex = owner->GetComponent<TextureComponent>())
            tex->SetTexture("Pooka.png");

        // Disable the enemy hitbox – Pooka no longer damages the player
        if (auto* hb = owner->GetComponent<HitboxComponent>())
            hb->SetEnabled(false);
    }

    std::unique_ptr<PookaState> PookaInflatingState::Update(GameObject* /*owner*/)
    {
        if (m_Released)
            return std::make_unique<PookaWalkingState>(m_pGridObject);
        return nullptr;
    }

    void PookaInflatingState::OnExit(GameObject* owner)
    {
        // Re-enable the hitbox so the Pooka can damage the player again
        if (auto* hb = owner->GetComponent<HitboxComponent>())
            hb->SetEnabled(true);
    }
}