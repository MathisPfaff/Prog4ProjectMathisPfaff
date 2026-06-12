#pragma once
#include "BaseComponent.h"
#include <glm/glm.hpp>
#include <vector>

namespace dae
{
    enum class HitboxType
    {
        Player,
        Enemy,
        Projectile,
        Pump,
        Fire      // Fygar fire breath – damages Player only
    };

    class HitboxComponent final : public BaseComponent
    {
    public:
        HitboxComponent(GameObject* owner, float width, float height, HitboxType type);
        ~HitboxComponent() override = default;

        void FixedUpdate(float) override {}
        void Update() override;
        void LateUpdate() override {}
        void Render() const override;

        bool Intersects(const HitboxComponent* other) const;

        glm::vec2 GetCenter() const;
        glm::vec2 GetSize() const { return glm::vec2(m_Width, m_Height); }
        HitboxType GetType() const { return m_Type; }

        void SetSize(float width, float height) { m_Width = width; m_Height = height; }
        void SetOffset(float x, float y) { m_Offset = glm::vec2(x, y); }

        void SetEnabled(bool enabled) { m_Enabled = enabled; }
        bool IsEnabled() const { return m_Enabled; }

        void SetCanDamage(bool canDamage) { m_CanDamage = canDamage; }
        bool CanDamage() const { return m_CanDamage; }

        static const std::vector<HitboxComponent*>& GetAllHitboxes() { return s_AllHitboxes; }

    private:
        float      m_Width{};
        float      m_Height{};
        glm::vec2  m_Offset{};
        HitboxType m_Type{};
        bool       m_Enabled{ true };
        bool       m_CanDamage{ true };

        static std::vector<HitboxComponent*> s_AllHitboxes;
    };
}