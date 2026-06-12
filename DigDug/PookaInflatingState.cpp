#include "PookaInflatingState.h"
#include "PookaWalkingState.h"
#include "GameObject.h"
#include "TextureComponent.h"
#include "HitboxComponent.h"
#include "GameTime.h"
#include <memory>
#include <algorithm>

namespace dae
{
    PookaInflatingState::PookaInflatingState(GameObject* pGridObject, float inheritedInflateLevel)
        : m_pGridObject(pGridObject)
        , m_InflateLevel(inheritedInflateLevel)
    {}

    void PookaInflatingState::OnEnter(GameObject* owner)
    {
        // TODO: swap for a dedicated inflate sprite ("PookaInflate.png")
        if (auto* tex = owner->GetComponent<TextureComponent>())
            tex->SetTexture("Pooka.png");

        // Enemy must not damage the player while inflating or deflating.
        // We use SetCanDamage (not SetEnabled) so the pump can still detect the
        // hitbox for reconnect purposes.
        if (auto* hb = owner->GetComponent<HitboxComponent>())
            hb->SetCanDamage(false);
    }

    std::unique_ptr<PookaState> PookaInflatingState::Update(GameObject* /*owner*/)
    {
        if (m_Mode == Mode::Deflating)
        {
            // m_InflateLevel doubles as the deflate timer (in seconds)
            m_InflateLevel -= GameTime::GetInstance().GetDeltaTime() * k_DeflateRate;
            if (m_InflateLevel <= 0.f)
            {
                m_InflateLevel = 0.f;
                return std::make_unique<PookaWalkingState>(m_pGridObject);
            }
        }
        // Inflating mode: level is driven externally by PumpComponent
        return nullptr;
    }

    void PookaInflatingState::OnExit(GameObject* owner)
    {
        // Restore damage when the enemy returns to walking state
        if (auto* hb = owner->GetComponent<HitboxComponent>())
            hb->SetCanDamage(true);
    }

    bool PookaInflatingState::AddInflate(float amount)
    {
        m_InflateLevel = std::min(m_InflateLevel + amount, k_MaxInflate);
        return m_InflateLevel >= k_MaxInflate;
    }
}