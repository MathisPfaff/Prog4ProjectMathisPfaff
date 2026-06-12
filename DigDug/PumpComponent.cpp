#include "PumpComponent.h"
#include "GameObject.h"
#include "GridComponent.h"
#include "HitboxComponent.h"
#include "PookaComponent.h"
#include "PlayerMovementComponent.h"
#include "GameTime.h"
#include "Renderer.h"
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <cmath>

namespace dae
{
    PumpComponent::PumpComponent(GameObject* owner, GameObject* pGridObject)
        : BaseComponent(owner)
        , m_pGridObject(pGridObject)
    {
        // Zero-sized pump hitbox; resized each frame while the beam is active
        m_pHitbox = owner->AddComponent<HitboxComponent>(0.f, 0.f, HitboxType::Pump);
    }

    // ── Public interface ────────────────────────────────────────────────────

    void PumpComponent::Fire()
    {
        if (m_State != PumpState::Idle) return;

        auto* owner = GetOwner();
        if (!owner) return;

        if (auto* mv = owner->GetComponent<PlayerMovementComponent>())
        {
            const glm::vec2 dir = mv->GetCurrentDirection();
            if (glm::length(dir) > 0.1f)
                m_FiringDirection = glm::normalize(dir);
        }

        m_CurrentLength = 0.f;
        m_State = PumpState::Extending;
    }

    void PumpComponent::InflatePulse()
    {
        // +1.5f per press – 3 presses kills the enemy
        if (m_State != PumpState::Stuck || !m_pStuckEnemy) return;

        if (m_pStuckEnemy->AddInflate(k_InflatePulse))
        {
            // Enemy just died; clean up beam immediately
            m_pStuckEnemy  = nullptr;
            m_CurrentLength = 0.f;
            m_State = PumpState::Idle;
        }
    }

    void PumpComponent::PumpHeld()
    {
        // Flag consumed in UpdateStuck every frame; no-ops outside Stuck state
        if (m_State == PumpState::Stuck)
            m_PumpHeldThisFrame = true;
    }

    void PumpComponent::ReleaseStuck()
    {
        if (m_State != PumpState::Stuck) return;

        if (m_pStuckEnemy)
        {
            // Enemy starts deflating; m_InflateLevel becomes the deflate timer
            m_pStuckEnemy->StartDeflating();
            m_pStuckEnemy = nullptr;
        }

        m_CurrentLength = 0.f;
        m_State = PumpState::Idle;
    }

    // ── Per-frame update ────────────────────────────────────────────────────

    void PumpComponent::Update()
    {
        const float dt = GameTime::GetInstance().GetDeltaTime();

        switch (m_State)
        {
        case PumpState::Extending:  UpdateExtending(dt);  break;
        case PumpState::Retracting: UpdateRetracting(dt); break;
        case PumpState::Stuck:      UpdateStuck(dt);      break;
        default: break;
        }

        // Always clear the held flag after it has been consumed (or ignored)
        m_PumpHeldThisFrame = false;

        UpdateHitbox();
    }

    void PumpComponent::UpdateExtending(float dt)
    {
        auto* grid = m_pGridObject ? m_pGridObject->GetComponent<GridComponent>() : nullptr;
        if (!grid) { m_State = PumpState::Idle; return; }

        const bool  horiz     = std::abs(m_FiringDirection.x) > 0.5f;
        const float cellSize  = horiz ? grid->GetCellWidth() : grid->GetCellHeight();
        const float maxLength = k_MaxCells * cellSize;

        const float newLength = std::min(m_CurrentLength + k_ExtendSpeed * dt, maxLength);

        // Tip in grid-relative space
        const glm::vec3 gridOrigin  = m_pGridObject->GetWorldPosition();
        const glm::vec3 playerWorld = GetOwner()->GetWorldPosition();
        const glm::vec2 relOrigin   = { playerWorld.x - gridOrigin.x,
                                        playerWorld.y - gridOrigin.y };
        const glm::vec2 newTipRel   = relOrigin + m_FiringDirection * newLength;

        if (IsSubCellBlocked(newTipRel.x, newTipRel.y))
        {
            m_State = PumpState::Retracting;
            return;
        }

        m_CurrentLength = newLength;

        // Sync hitbox before intersection test so Intersects() is accurate
        UpdateHitbox();

        // ── Enemy hit / reconnect scan ──────────────────────────────────────
        for (auto* hb : HitboxComponent::GetAllHitboxes())
        {
            if (!hb || hb == m_pHitbox)            continue;
            if (hb->GetType() != HitboxType::Enemy) continue;

            auto* enemyObj  = hb->GetOwner();
            if (!enemyObj) continue;

            auto* pookaComp = enemyObj->GetComponent<PookaComponent>();
            // IsPumpable() = false for ghost + actively-inflating enemy
            if (!pookaComp || !pookaComp->IsPumpable()) continue;

            if (!m_pHitbox->Intersects(hb)) continue;

            // Latch beam to enemy (fresh hit or reconnect after deflation started)
            m_pStuckEnemy = pookaComp;
            pookaComp->BeginInflating();
            m_State = PumpState::Stuck;
            return;
        }
        // ───────────────────────────────────────────────────────────────────

        if (m_CurrentLength >= maxLength)
            m_State = PumpState::Retracting;
    }

    void PumpComponent::UpdateRetracting(float dt)
    {
        m_CurrentLength -= k_RetractSpeed * dt;
        if (m_CurrentLength <= 0.f)
        {
            m_CurrentLength = 0.f;
            m_State = PumpState::Idle;
        }
    }

    void PumpComponent::UpdateStuck(float dt)
    {
        // Safety: if the enemy object was destroyed externally, clean up
        if (!m_pStuckEnemy)
        {
            m_CurrentLength = 0.f;
            m_State = PumpState::Idle;
            return;
        }

        // Held-button continuous inflation (1.5 units/s → 3 s to kill from 0)
        if (m_PumpHeldThisFrame)
        {
            if (m_pStuckEnemy->AddInflate(k_InflateHeldRate * dt))
            {
                // Enemy died from continuous pumping
                m_pStuckEnemy  = nullptr;
                m_CurrentLength = 0.f;
                m_State = PumpState::Idle;
            }
        }
    }

    // ── Helpers ─────────────────────────────────────────────────────────────

    bool PumpComponent::IsSubCellBlocked(float relX, float relY) const
    {
        auto* grid = m_pGridObject ? m_pGridObject->GetComponent<GridComponent>() : nullptr;
        if (!grid) return true;

        int subCol{}, subRow{};
        if (!grid->WorldToSubCell(relX, relY, subCol, subRow)) return true;

        if (grid->IsGroundSubRow(subRow)) return false;

        return !grid->IsSubCellDug(subCol, subRow);
    }

    void PumpComponent::UpdateHitbox()
    {
        if (!m_pHitbox) return;

        if (m_State == PumpState::Idle || m_CurrentLength <= 0.f)
        {
            m_pHitbox->SetSize(0.f, 0.f);
            m_pHitbox->SetOffset(0.f, 0.f);
            return;
        }

        auto* grid = m_pGridObject ? m_pGridObject->GetComponent<GridComponent>() : nullptr;
        if (!grid) return;

        const float halfLen   = m_CurrentLength * 0.5f;
        const bool  horiz     = std::abs(m_FiringDirection.x) > 0.5f;
        const float thickness = horiz ? grid->GetSubCellHeight() : grid->GetSubCellWidth();

        if (horiz)
        {
            m_pHitbox->SetSize(m_CurrentLength, thickness);
            m_pHitbox->SetOffset(m_FiringDirection.x * halfLen, 0.f);
        }
        else
        {
            m_pHitbox->SetSize(thickness, m_CurrentLength);
            m_pHitbox->SetOffset(0.f, m_FiringDirection.y * halfLen);
        }
    }

    void PumpComponent::Render() const
    {
        if (m_State == PumpState::Idle || m_CurrentLength <= 0.f) return;

        auto* owner = GetOwner();
        if (!owner) return;

        const glm::vec3 wp = owner->GetWorldPosition();
        const float x1 = wp.x;
        const float y1 = wp.y;
        const float x2 = wp.x + m_FiringDirection.x * m_CurrentLength;
        const float y2 = wp.y + m_FiringDirection.y * m_CurrentLength;

        SDL_Renderer* r = Renderer::GetInstance().GetSDLRenderer();

        // Green = extending/retracting; yellow = stuck on enemy
        if (m_State == PumpState::Stuck)
            SDL_SetRenderDrawColor(r, 255, 220, 0, 255);
        else
            SDL_SetRenderDrawColor(r, 0, 220, 0, 255);

        const bool horiz = std::abs(m_FiringDirection.x) > 0.5f;
        for (int i = -2; i <= 2; ++i)
        {
            const float ox = horiz ? 0.f : static_cast<float>(i);
            const float oy = horiz ? static_cast<float>(i) : 0.f;
            SDL_RenderLine(r, x1 + ox, y1 + oy, x2 + ox, y2 + oy);
        }
    }
}