#pragma once
#include "BaseComponent.h"
#include "Subject.h"
#include "Observer.h"
#include "Hash.h"
#include "HealthComponent.h"

namespace ScoreEvent
{
    constexpr unsigned int Added = make_sdbm_hash("ScoreAdded");
}

namespace dae
{
    enum class EnemyType
    {
        Pooka,
        Fygar
    };

    class ScoreComponent final : public BaseComponent, public Observer
    {
    public:
        explicit ScoreComponent(GameObject* owner, int deathBonus = 0)
            : BaseComponent(owner), m_DeathBonus(deathBonus) {}
        virtual ~ScoreComponent() override = default;
        ScoreComponent(const ScoreComponent&)            = delete;
        ScoreComponent(ScoreComponent&&)                 = delete;
        ScoreComponent& operator=(const ScoreComponent&) = delete;
        ScoreComponent& operator=(ScoreComponent&&)      = delete;

        void FixedUpdate(float) override {}
        void Update()           override {}
        void LateUpdate()       override {}
        void Render() const     override {}

        void OnNotify(BaseComponent*, unsigned int eventID) override
        {
            if (eventID == HealthEvent::Died)
                AddScore(m_DeathBonus);
        }

        void AddScore(int points)
        {
            m_Score += points;
            m_Subject.Notify(this, ScoreEvent::Added);
        }

        // Awards the correct Dig Dug kill score.
        // enemyRow    : grid row the enemy was on when it died (0 = ground, 1-15 = underground)
        // fygarSameRow: true only for Fygar kills where the player shares the same row
        void AddScoreForKill(EnemyType type, int enemyRow, bool fygarSameRowAsPlayer);

        int GetScore() const { return m_Score; }

        void AddObserver(Observer* observer)    { m_Subject.AddObserver(observer); }
        void RemoveObserver(Observer* observer) { m_Subject.RemoveObserver(observer); }

    private:
        Subject m_Subject;
        int m_Score{ 0 };
        int m_DeathBonus{ 0 };

        // Row 0 (ground) and rows 1-4 → level 1 (200 pts for Pooka)
        // Rows  5- 8 → level 2 (300), rows  9-12 → level 3 (400), rows 13-15 → level 4 (500)
        static int GetUndergroundLevel(int row);
        static int CalculateKillScore(EnemyType type, int enemyRow, bool fygarSameRowAsPlayer);
    };
}