#include "FireBreathComponent.h"
#include "GameObject.h"
#include "GridComponent.h"
#include "HitboxComponent.h"
#include "GameTime.h"
#include "Renderer.h"
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <cmath>

namespace dae
{
    FireBreathComponent::FireBreathComponent(GameObject* owner, GameObject* pGridObject)
        : BaseComponent(owner)
        , m_pGridObject(pGridObject)
    {
        // Zero-sized fire hitbox; resized while the flame is active (same pattern as PumpComponent)
        m_pFireHitbox = owner->AddComponent<HitboxComponent>(0.f, 0.f, HitboxType::Fire);
        m_pFireHitbox->SetCanDamage(false); // off until Fire() is called
    }

    // ── Public interface ────────────────────────────────────────────────────

    void FireBreathComponent::Fire()
    {
        if (m_Active) return;

        m_FiringDirection = m_FacingDirection;
        m_CurrentLength = 0.f;
        m_HoldTimer = 0.f;
        m_FireState = FireState::Extending;
        m_Active = true;
        m_pFireHitbox->SetCanDamage(true);
    }

    void FireBreathComponent::StopFire()
    {
        m_Active = false;
        m_FireState = FireState::Idle;
        m_CurrentLength = 0.f;
        m_pFireHitbox->SetCanDamage(false);
        UpdateHitbox(); // zero the hitbox immediately
    }

    // ── Per-frame update ────────────────────────────────────────────────────

    void FireBreathComponent::Update()
    {
        if (!m_Active) return;

        auto* grid = m_pGridObject ? m_pGridObject->GetComponent<GridComponent>() : nullptr;
        if (!grid) { StopFire(); return; }

        const float dt = GameTime::GetInstance().GetDeltaTime();
        const bool  horiz = std::abs(m_FiringDirection.x) > 0.5f;
        const float cellSize = horiz ? grid->GetCellWidth() : grid->GetCellHeight();
        const float maxLen = k_MaxCells * cellSize;

        switch (m_FireState)
        {
        case FireState::Extending:
            m_CurrentLength += k_ExtendSpeed * dt;
            if (m_CurrentLength >= maxLen)
            {
                m_CurrentLength = maxLen;
                m_FireState = FireState::Holding;
                m_HoldTimer = 0.f;
            }
            break;

        case FireState::Holding:
            m_HoldTimer += dt;
            if (m_HoldTimer >= k_HoldTime)
                m_FireState = FireState::Retracting;
            break;

        case FireState::Retracting:
            m_CurrentLength -= k_RetractSpeed * dt;
            if (m_CurrentLength <= 0.f)
            {
                StopFire();
                return;
            }
            break;

        default: break;
        }

        UpdateHitbox();
    }

    // ── Helpers ─────────────────────────────────────────────────────────────

    void FireBreathComponent::UpdateHitbox()
    {
        if (!m_pFireHitbox) return;

        if (!m_Active || m_CurrentLength <= 0.f)
        {
            m_pFireHitbox->SetSize(0.f, 0.f);
            m_pFireHitbox->SetOffset(0.f, 0.f);
            return;
        }

        auto* grid = m_pGridObject ? m_pGridObject->GetComponent<GridComponent>() : nullptr;
        if (!grid) return;

        const float halfLen = m_CurrentLength * 0.5f;
        const bool  horiz = std::abs(m_FiringDirection.x) > 0.5f;
        const float thickness = horiz ? grid->GetCellHeight() : grid->GetCellWidth();

        if (horiz)
        {
            m_pFireHitbox->SetSize(m_CurrentLength, thickness);
            m_pFireHitbox->SetOffset(m_FiringDirection.x * halfLen, 0.f);
        }
        else
        {
            m_pFireHitbox->SetSize(thickness, m_CurrentLength);
            m_pFireHitbox->SetOffset(0.f, m_FiringDirection.y * halfLen);
        }
    }

    void FireBreathComponent::Render() const
    {
        if (!m_Active || m_CurrentLength <= 0.f) return;

        auto* owner = GetOwner();
        if (!owner) return;

        const glm::vec3 wp = owner->GetWorldPosition();
        const float x1 = wp.x;
        const float y1 = wp.y;
        const float x2 = wp.x + m_FiringDirection.x * m_CurrentLength;
        const float y2 = wp.y + m_FiringDirection.y * m_CurrentLength;

        SDL_Renderer* r = Renderer::GetInstance().GetSDLRenderer();

        // Colour shifts orange → yellow while holding
        if (m_FireState == FireState::Holding)
            SDL_SetRenderDrawColor(r, 255, 220, 0, 255);
        else
            SDL_SetRenderDrawColor(r, 255, 80, 0, 255);

        const bool horiz = std::abs(m_FiringDirection.x) > 0.5f;
        for (int i = -3; i <= 3; ++i)
        {
            const float ox = horiz ? 0.f : static_cast<float>(i * 2);
            const float oy = horiz ? static_cast<float>(i * 2) : 0.f;
            SDL_RenderLine(r, x1 + ox, y1 + oy, x2 + ox, y2 + oy);
        }
    }
}