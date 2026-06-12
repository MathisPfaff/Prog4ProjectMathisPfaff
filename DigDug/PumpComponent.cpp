#include "PumpComponent.h"
#include "GameObject.h"
#include "GridComponent.h"
#include "HitboxComponent.h"
#include "PookaComponent.h"
#include "FygarComponent.h"
#include "PlayerMovementComponent.h"
#include "ScoreComponent.h"
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

    // ── Generic stuck-enemy helpers ─────────────────────────────────────────

    bool PumpComponent::HasStuckEnemy() const
    {
        return m_pStuckEnemy != nullptr || m_pStuckEnemy_Fygar != nullptr;
    }

    bool PumpComponent::StuckEnemyAddInflate(float amount)
    {
        if (m_pStuckEnemy)      return m_pStuckEnemy->AddInflate(amount);
        if (m_pStuckEnemy_Fygar) return m_pStuckEnemy_Fygar->AddInflate(amount);
        return false;
    }

    void PumpComponent::StuckEnemyStartDeflating()
    {
        if (m_pStuckEnemy)       m_pStuckEnemy->StartDeflating();
        if (m_pStuckEnemy_Fygar) m_pStuckEnemy_Fygar->StartDeflating();
    }

    void PumpComponent::ClearStuckEnemy()
    {
        m_pStuckEnemy      = nullptr;
        m_pStuckEnemy_Fygar = nullptr;
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
        if (m_State != PumpState::Stuck || !HasStuckEnemy()) return;

        if (StuckEnemyAddInflate(k_InflatePulse))
        {
            TryAwardKillScore();   // award score while pointers are still valid
            ClearStuckEnemy();
            m_CurrentLength = 0.f;
            m_State = PumpState::Idle;
        }
    }

    void PumpComponent::PumpHeld()
    {
        if (m_State == PumpState::Stuck)
            m_PumpHeldThisFrame = true;
    }

    void PumpComponent::ReleaseStuck()
    {
        if (m_State != PumpState::Stuck) return;

        StuckEnemyStartDeflating();
        ClearStuckEnemy();

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
        UpdateHitbox();

        // ── Enemy hit / reconnect scan ──────────────────────────────────────
        for (auto* hb : HitboxComponent::GetAllHitboxes())
        {
            if (!hb || hb == m_pHitbox)            continue;
            if (hb->GetType() != HitboxType::Enemy) continue;

            auto* enemyObj = hb->GetOwner();
            if (!enemyObj) continue;

            if (auto* pooka = enemyObj->GetComponent<PookaComponent>())
            {
                if (!pooka->IsPumpable() || !m_pHitbox->Intersects(hb)) continue;
                m_pStuckEnemy = pooka;
                pooka->BeginInflating();
                m_State = PumpState::Stuck;
                return;
            }
            if (auto* fygar = enemyObj->GetComponent<FygarComponent>())
            {
                if (!fygar->IsPumpable() || !m_pHitbox->Intersects(hb)) continue;
                m_pStuckEnemy_Fygar = fygar;
                fygar->BeginInflating();
                m_State = PumpState::Stuck;
                return;
            }
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
        if (!HasStuckEnemy())
        {
            m_CurrentLength = 0.f;
            m_State = PumpState::Idle;
            return;
        }

        if (m_PumpHeldThisFrame)
        {
            if (StuckEnemyAddInflate(k_InflateHeldRate * dt))
            {
                TryAwardKillScore();   // award score while pointers are still valid
                ClearStuckEnemy();
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

    void PumpComponent::TryAwardKillScore()
    {
        if (!GetOwner()) return;
        auto* scoreComp = GetOwner()->GetComponent<ScoreComponent>();
        if (!scoreComp) return;

        auto* grid = m_pGridObject ? m_pGridObject->GetComponent<GridComponent>() : nullptr;
        if (!grid) return;

        const glm::vec3 gridOrigin  = m_pGridObject->GetWorldPosition();
        const glm::vec3 playerWorld = GetOwner()->GetWorldPosition();

        int playerCol{}, playerRow{};
        grid->WorldToCell(playerWorld.x - gridOrigin.x,
                          playerWorld.y - gridOrigin.y,
                          playerCol, playerRow);

        if (m_pStuckEnemy)  // Pooka kill
        {
            const glm::vec3 enemyWorld = m_pStuckEnemy->GetOwner()->GetWorldPosition();
            int enemyCol{}, enemyRow{};
            grid->WorldToCell(enemyWorld.x - gridOrigin.x,
                              enemyWorld.y - gridOrigin.y,
                              enemyCol, enemyRow);
            scoreComp->AddScoreForKill(EnemyType::Pooka, enemyRow, false);
        }
        else if (m_pStuckEnemy_Fygar)  // Fygar kill
        {
            const glm::vec3 enemyWorld = m_pStuckEnemy_Fygar->GetOwner()->GetWorldPosition();
            int enemyCol{}, enemyRow{};
            grid->WorldToCell(enemyWorld.x - gridOrigin.x,
                              enemyWorld.y - gridOrigin.y,
                              enemyCol, enemyRow);
            const bool sameRow = (playerRow == enemyRow);
            scoreComp->AddScoreForKill(EnemyType::Fygar, enemyRow, sameRow);
        }
    }
}