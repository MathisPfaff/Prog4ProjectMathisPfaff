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
        m_pHitbox = owner->AddComponent<HitboxComponent>(0.f, 0.f, HitboxType::Pump);
    }

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

    void PumpComponent::ReleaseStuck()
    {
        if (m_State != PumpState::Stuck) return;

        if (m_pStuckEnemy)
        {
            m_pStuckEnemy->ReleaseInflating(); // sets m_Released on PookaInflatingState
            m_pStuckEnemy = nullptr;
        }

        m_CurrentLength = 0.f;
        m_State = PumpState::Idle;
    }

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

        UpdateHitbox();
    }

    void PumpComponent::UpdateExtending(float dt)
    {
        auto* grid = m_pGridObject ? m_pGridObject->GetComponent<GridComponent>() : nullptr;
        if (!grid) { m_State = PumpState::Idle; return; }

        const bool  horiz    = std::abs(m_FiringDirection.x) > 0.5f;
        const float cellSize = horiz ? grid->GetCellWidth() : grid->GetCellHeight();
        const float maxLength = k_MaxCells * cellSize;

        const float newLength = std::min(m_CurrentLength + k_ExtendSpeed * dt, maxLength);

        // Tip position in grid-relative space
        const glm::vec3 gridOrigin  = m_pGridObject->GetWorldPosition();
        const glm::vec3 playerWorld = GetOwner()->GetWorldPosition();
        const glm::vec2 relOrigin{ playerWorld.x - gridOrigin.x,
                                   playerWorld.y - gridOrigin.y };
        const glm::vec2 newTipRel = relOrigin + m_FiringDirection * newLength;

        if (IsSubCellBlocked(newTipRel.x, newTipRel.y))
        {
            m_State = PumpState::Retracting;
            return;
        }

        m_CurrentLength = newLength;

        // ── Enemy hit detection ─────────────────────────────────────────────
        // Sync the pump hitbox to the new length so Intersects() is accurate.
        UpdateHitbox();

        for (auto* hb : HitboxComponent::GetAllHitboxes())
        {
            if (!hb || hb == m_pHitbox)           continue;
            if (hb->GetType() != HitboxType::Enemy) continue;
            if (!hb->IsEnabled())                   continue; // already inflating

            auto* enemyObj  = hb->GetOwner();
            if (!enemyObj) continue;

            auto* pookaComp = enemyObj->GetComponent<PookaComponent>();
            // IsPumpable() returns false for ghost state – ghosts are immune
            if (!pookaComp || !pookaComp->IsPumpable()) continue;

            if (!m_pHitbox->Intersects(hb)) continue;

            // Beam hit a pumpable enemy – freeze the beam and inflate the enemy
            m_pStuckEnemy = pookaComp;
            pookaComp->BeginInflating();
            m_State = PumpState::Stuck;
            return; // skip the max-length retract check
        }
        // ────────────────────────────────────────────────────────────────────

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

    void PumpComponent::UpdateStuck(float)
    {
        // Beam is frozen at m_CurrentLength until ReleaseStuck() is called.
        // Safety: if the enemy pointer was cleared externally, auto-release.
        if (!m_pStuckEnemy)
        {
            m_CurrentLength = 0.f;
            m_State = PumpState::Idle;
        }
    }

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

        // Bright green while extending/retracting; yellow while stuck to an enemy
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