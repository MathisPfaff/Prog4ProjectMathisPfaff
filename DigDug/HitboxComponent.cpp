#include "HitboxComponent.h"
#include "GameObject.h"
#include "Renderer.h"
#include "HealthComponent.h"
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
        // Register this hitbox
        s_AllHitboxes.push_back(this);
    }

    void HitboxComponent::Update()
    {
        // Collision detection happens here
        auto* owner = GetOwner();
        if (!owner) return;

        // Check collisions with other hitboxes
        for (auto* other : s_AllHitboxes)
        {
            if (other == this) continue;
            if (!other->GetOwner()) continue;

            // Check if this is a player-enemy collision
            if ((m_Type == HitboxType::Player && other->m_Type == HitboxType::Enemy) ||
                (m_Type == HitboxType::Enemy && other->m_Type == HitboxType::Player))
            {
                if (Intersects(other))
                {
                    // Find which one is the player and notify damage
                    auto* playerOwner = (m_Type == HitboxType::Player) ? owner : other->GetOwner();
                    auto* healthComp = playerOwner->GetComponent<HealthComponent>();
                    if (healthComp)
                    {
                        healthComp->TakeDamage(1);
                    }
                }
            }
        }
    }

    void HitboxComponent::Render() const
    {
#ifdef _DEBUG
        // Draw debug hitbox
        auto* owner = GetOwner();
        if (!owner) return;

        const glm::vec3 worldPos = owner->GetWorldPosition();
        const glm::vec2 center = GetCenter();

        const float left = center.x - m_Width * 0.5f;
        const float top = center.y - m_Height * 0.5f;

        SDL_FRect rect{ left, top, m_Width, m_Height };

        // Different colors for different types
        SDL_Color color = (m_Type == HitboxType::Player) ? SDL_Color{ 0, 255, 0, 128 } : SDL_Color{ 255, 0, 0, 128 };

        auto& renderer = Renderer::GetInstance();
        SDL_Renderer* sdlRenderer = renderer.GetSDLRenderer();
        SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
        SDL_RenderRect(sdlRenderer, &rect);
#endif
    }

    bool HitboxComponent::Intersects(const HitboxComponent* other) const
    {
        if (!other) return false;

        const glm::vec2 thisCenter = GetCenter();
        const glm::vec2 otherCenter = other->GetCenter();

        const float thisLeft = thisCenter.x - m_Width * 0.5f;
        const float thisRight = thisCenter.x + m_Width * 0.5f;
        const float thisTop = thisCenter.y - m_Height * 0.5f;
        const float thisBottom = thisCenter.y + m_Height * 0.5f;

        const float otherLeft = otherCenter.x - other->m_Width * 0.5f;
        const float otherRight = otherCenter.x + other->m_Width * 0.5f;
        const float otherTop = otherCenter.y - other->m_Height * 0.5f;
        const float otherBottom = otherCenter.y + other->m_Height * 0.5f;

        return !(thisRight < otherLeft || thisLeft > otherRight ||
            thisBottom < otherTop || thisTop > otherBottom);
    }

    glm::vec2 HitboxComponent::GetCenter() const
    {
        auto* owner = GetOwner();
        if (!owner) return glm::vec2{};

        const glm::vec3 worldPos = owner->GetWorldPosition();
        return glm::vec2(worldPos.x, worldPos.y) + m_Offset;
    }
}