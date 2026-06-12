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
    {
        m_CurrentStage = GetCurrentStage();
    }

    void PookaInflatingState::OnEnter(GameObject* owner)
    {
        // Set inflate texture once
        if (auto* tex = owner->GetComponent<TextureComponent>())
            tex->SetTexture("PookaInflateStage0.png");

        // Enemy must not damage the player while inflating or deflating.
        // We use SetCanDamage (not SetEnabled) so the pump can still detect the
        // hitbox for reconnect purposes.
        if (auto* hb = owner->GetComponent<HitboxComponent>())
            hb->SetCanDamage(false);

        // Set initial scale based on inherited inflate level (handles reconnects)
        UpdateScale(owner);
    }

    std::unique_ptr<PookaState> PookaInflatingState::Update(GameObject* owner)
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

        // Check if we crossed a stage threshold and need to update scale
        const int newStage = GetCurrentStage();
        if (newStage != m_CurrentStage)
        {
            m_CurrentStage = newStage;
            UpdateScale(owner);
        }

        return nullptr;
    }

    void PookaInflatingState::OnExit(GameObject* owner)
    {
        // Restore damage when the enemy returns to walking state
        if (auto* hb = owner->GetComponent<HitboxComponent>())
            hb->SetCanDamage(true);

        // Restore normal walking texture and scale
        if (auto* tex = owner->GetComponent<TextureComponent>())
        {
            tex->SetTexture("Pooka.png");
            tex->SetScale(k_BaseScale);
        }
    }

    bool PookaInflatingState::AddInflate(float amount)
    {
        m_InflateLevel = std::min(m_InflateLevel + amount, k_MaxInflate);
        return m_InflateLevel >= k_MaxInflate;
    }

    int PookaInflatingState::GetCurrentStage() const
    {
        // Stage thresholds: 0.0-1.5 = stage 0, 1.5-3.0 = stage 1, 3.0-4.5 = stage 2, 4.5 = stage 3
        if (m_InflateLevel >= 4.5f) return 3;
        if (m_InflateLevel >= 3.0f) return 2;
        if (m_InflateLevel >= 1.5f) return 1;
        return 0;
    }

    void PookaInflatingState::UpdateScale(GameObject* owner)
    {
        if (!owner) return;

        auto* tex = owner->GetComponent<TextureComponent>();
        if (!tex) return;

        // Only update scale based on current stage
        tex->SetScale(k_StageScales[m_CurrentStage]);
    }
}