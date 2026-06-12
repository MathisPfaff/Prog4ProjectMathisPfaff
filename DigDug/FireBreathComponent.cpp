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
        m_pFireHitbox = owner->AddComponent<HitboxComponent>(0.f, 0.f, HitboxType::Fire);
        m_pFireHitbox->SetCanDamage(false);
    }

    // ── Public interface ────────────────────────────────────────────────────

    void FireBreathComponent::Fire()
    {
        if (m_Active) return;

        // Fire breath is ALWAYS horizontal – snap vertical or zero facing to right
        if (std::abs(m_FacingDirection.x) >= std::abs(m_FacingDirection.y)
            && m_FacingDirection.x != 0.f)
            m_FiringDirection = { m_FacingDirection.x > 0.f ? 1.f : -1.f, 0.f };
        else
            m_FiringDirection = { 1.f, 0.f };  // fallback: right

        m_CurrentLength = 0.f;
        m_HoldTimer     = 0.f;
        m_FireState     = FireState::Extending;
        m_Active        = true;
        m_pFireHitbox->SetCanDamage(true);
    }

    void FireBreathComponent::StopFire()
    {
        m_Active        = false;
        m_FireState     = FireState::Idle;
        m_CurrentLength = 0.f;
        m_pFireHitbox->SetCanDamage(false);
        UpdateHitbox();
    }

    // ── Per-frame update ────────────────────────────────────────────────────

    void FireBreathComponent::Update()
    {
        if (!m_Active) return;

        auto* grid = m_pGridObject ? m_pGridObject->GetComponent<GridComponent>() : nullptr;
        if (!grid) { StopFire(); return; }

        const float dt       = GameTime::GetInstance().GetDeltaTime();
        const float cellSize = grid->GetCellWidth();           // always horizontal now
        const float maxLen   = k_MaxCells * cellSize;

        switch (m_FireState)
        {
        case FireState::Extending:
        {
            const float candidate = m_CurrentLength + k_ExtendSpeed * dt;

            // Compute the tip in grid-relative space – identical to PumpComponent
            const glm::vec3 gridOrigin = m_pGridObject->GetWorldPosition();
            const glm::vec3 ownerWorld = GetOwner()->GetWorldPosition();
            const glm::vec2 relOrigin  = { ownerWorld.x - gridOrigin.x,
                                           ownerWorld.y - gridOrigin.y };
            const glm::vec2 newTipRel  = relOrigin + m_FiringDirection * candidate;

            if (IsFireBlocked(newTipRel.x, newTipRel.y))
            {
                // Hit undug dirt – stop extending at current length, start holding
                // (m_CurrentLength is NOT advanced so fire sits just before the wall)
                m_FireState = FireState::Holding;
                m_HoldTimer = 0.f;
            }
            else if (candidate >= maxLen)
            {
                m_CurrentLength = maxLen;
                m_FireState     = FireState::Holding;
                m_HoldTimer     = 0.f;
            }
            else
            {
                m_CurrentLength = candidate;
            }
            break;
        }

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

    bool FireBreathComponent::IsFireBlocked(float relX, float relY) const
    {
        // Identical contract to PumpComponent::IsSubCellBlocked:
        //   ground row sub-rows are never blocked (above-ground area)
        //   any other undug subcell blocks the fire
        auto* grid = m_pGridObject ? m_pGridObject->GetComponent<GridComponent>() : nullptr;
        if (!grid) return true;

        int subCol{}, subRow{};
        if (!grid->WorldToSubCell(relX, relY, subCol, subRow)) return true;
        if (grid->IsGroundSubRow(subRow)) return false;
        return !grid->IsSubCellDug(subCol, subRow);
    }

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

        // Fire is always horizontal – thickness spans the full cell height
        const float halfLen   = m_CurrentLength * 0.5f;
        const float thickness = grid->GetCellHeight();

        m_pFireHitbox->SetSize(m_CurrentLength, thickness);
        m_pFireHitbox->SetOffset(m_FiringDirection.x * halfLen, 0.f);
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
        const float y2 = wp.y;  // always horizontal, y never changes

        SDL_Renderer* r = Renderer::GetInstance().GetSDLRenderer();

        if (m_FireState == FireState::Holding)
            SDL_SetRenderDrawColor(r, 255, 220, 0, 255);  // yellow at full extension
        else
            SDL_SetRenderDrawColor(r, 255, 80, 0, 255);   // orange while extending/retracting

        for (int i = -3; i <= 3; ++i)
        {
            const float oy = static_cast<float>(i * 2);
            SDL_RenderLine(r, x1, y1 + oy, x2, y2 + oy);
        }
    }
}