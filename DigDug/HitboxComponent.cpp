#include "HitboxComponent.h"
#include "GameObject.h"
#include "Renderer.h"
#include "HealthComponent.h"
#include "GameTime.h"
#include <SDL3/SDL.h>
#include <algorithm>

namespace dae
{
    std::vector<HitboxComponent*> HitboxComponent::s_AllHitboxes{};

    HitboxComponent::HitboxComponent(GameObject* owner, float width, float height, HitboxType type)
        : BaseComponent(owner)
        , m_Width(width)
        , m_Height(height)
        , m_Type(type)
    {
        s_AllHitboxes.push_back(this);
    }

    HitboxComponent::~HitboxComponent()
    {
        auto it = std::find(s_AllHitboxes.begin(), s_AllHitboxes.end(), this);
        if (it != s_AllHitboxes.end())
            s_AllHitboxes.erase(it);
    }

    void HitboxComponent::Update()
    {
        if (!m_Enabled) return;

        // Tick the per-hitbox cooldown
        if (m_DamageCooldownTimer > 0.f)
        {
            m_DamageCooldownTimer -= GameTime::GetInstance().GetDeltaTime();
            return; // still cooling down – don't deal damage this frame
        }

        if (!m_CanDamage) return;

        auto* owner = GetOwner();
        if (!owner) return;

        for (auto* other : s_AllHitboxes)
        {
            if (other == this) continue;
            if (!other->GetOwner()) continue;
            if (!other->IsEnabled()) continue;

            // ── Enemy body hits Player ──────────────────────────────────────
            // Only the ENEMY side triggers damage so the pair is only processed once.
            if (m_Type == HitboxType::Enemy && other->m_Type == HitboxType::Player)
            {
                if (!m_CanDamage) continue;

                if (Intersects(other))
                {
                    auto* playerOwner = other->GetOwner();
                    if (auto* health = playerOwner->GetComponent<HealthComponent>())
                    {
                        health->TakeDamage(1);
                        // Start cooldown on this enemy hitbox so it can't hit again
                        // until the player has had time to respawn
                        m_DamageCooldownTimer = k_DamageCooldown;
                    }
                }
            }

            // ── Fire breath hits Player ─────────────────────────────────────
            // Only the FIRE side triggers damage.
            if (m_Type == HitboxType::Fire && other->m_Type == HitboxType::Player)
            {
                if (!m_CanDamage) continue;

                if (Intersects(other))
                {
                    auto* playerOwner = other->GetOwner();
                    if (auto* health = playerOwner->GetComponent<HealthComponent>())
                    {
                        health->TakeDamage(1);
                        m_DamageCooldownTimer = k_DamageCooldown;
                    }
                }
            }
        }
    }

    void HitboxComponent::Render() const
    {
#ifdef _DEBUG
        auto* owner = GetOwner();
        if (!owner) return;

        const glm::vec2 center = GetCenter();
        const float left = center.x - m_Width * 0.5f;
        const float top  = center.y - m_Height * 0.5f;

        SDL_FRect rect{ left, top, m_Width, m_Height };

        SDL_Color color;
        if (m_Type == HitboxType::Fire)
            color = { 255, 80, 0, 180 };
        else if (!m_CanDamage)
            color = { 255, 165, 0, 128 };
        else
            color = (m_Type == HitboxType::Player)
                ? SDL_Color{ 0, 255, 0, 128 }
                : SDL_Color{ 255, 0, 0, 128 };

        SDL_Renderer* sdlRenderer = Renderer::GetInstance().GetSDLRenderer();
        SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
        SDL_RenderRect(sdlRenderer, &rect);
#endif
    }

    bool HitboxComponent::Intersects(const HitboxComponent* other) const
    {
        if (!other) return false;

        const glm::vec2 thisCenter  = GetCenter();
        const glm::vec2 otherCenter = other->GetCenter();

        const float thisLeft   = thisCenter.x - m_Width         * 0.5f;
        const float thisRight  = thisCenter.x + m_Width         * 0.5f;
        const float thisTop    = thisCenter.y - m_Height        * 0.5f;
        const float thisBottom = thisCenter.y + m_Height        * 0.5f;

        const float otherLeft   = otherCenter.x - other->m_Width  * 0.5f;
        const float otherRight  = otherCenter.x + other->m_Width  * 0.5f;
        const float otherTop    = otherCenter.y - other->m_Height * 0.5f;
        const float otherBottom = otherCenter.y + other->m_Height * 0.5f;

        return !(thisRight < otherLeft  || thisLeft > otherRight ||
                 thisBottom < otherTop  || thisTop  > otherBottom);
    }

    glm::vec2 HitboxComponent::GetCenter() const
    {
        auto* owner = GetOwner();
        if (!owner) return glm::vec2{};
        const glm::vec3 worldPos = owner->GetWorldPosition();
        return glm::vec2(worldPos.x, worldPos.y) + m_Offset;
    }
}