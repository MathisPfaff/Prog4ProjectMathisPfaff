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
    PookaInflatingState::PookaInflatingState(GameObject* pGridObject,
                                             float inheritedInflateLevel,
                                             std::string walkTexture,
                                             std::string inflateTexture)
        : m_pGridObject(pGridObject)
        , m_InflateLevel(inheritedInflateLevel)
        , m_WalkTexture(std::move(walkTexture))
        , m_InflateTexture(std::move(inflateTexture))
    {
        m_CurrentStage = GetCurrentStage();
    }

    void PookaInflatingState::OnEnter(GameObject* owner)
    {
        if (auto* tex = owner->GetComponent<TextureComponent>())
            tex->SetTexture(m_InflateTexture);

        if (auto* hb = owner->GetComponent<HitboxComponent>())
            hb->SetCanDamage(false);

        UpdateScale(owner);
    }

    std::unique_ptr<PookaState> PookaInflatingState::Update(GameObject* owner)
    {
        if (m_Mode == Mode::Deflating)
        {
            m_InflateLevel -= GameTime::GetInstance().GetDeltaTime() * k_DeflateRate;
            if (m_InflateLevel <= 0.f)
            {
                m_InflateLevel = 0.f;
                return std::make_unique<PookaWalkingState>(m_pGridObject, 10.f, 60.f,
                                                           m_WalkTexture, "PookaGhost.png");
            }
        }

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
        if (auto* hb = owner->GetComponent<HitboxComponent>())
            hb->SetCanDamage(true);

        if (auto* tex = owner->GetComponent<TextureComponent>())
        {
            tex->SetTexture(m_WalkTexture);
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
        tex->SetScale(k_StageScales[m_CurrentStage]);
    }
}