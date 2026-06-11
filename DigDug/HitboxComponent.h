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
        Projectile
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

        // Get all hitboxes in the scene for collision checking
        static const std::vector<HitboxComponent*>& GetAllHitboxes() { return s_AllHitboxes; }

    private:
        float m_Width{};
        float m_Height{};
        glm::vec2 m_Offset{}; // Offset from GameObject position
        HitboxType m_Type{};

        // Static registry of all hitboxes for collision detection
        static std::vector<HitboxComponent*> s_AllHitboxes;
    };
}