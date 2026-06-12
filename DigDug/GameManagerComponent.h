#pragma once
#include "BaseComponent.h"
#include "Observer.h"
#include "HealthComponent.h"
#include "LivesDisplayObserver.h"
#include "ScoreDisplayObserver.h"
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace dae
{
    class GameState;

    struct SpawnInfo
    {
        int         gridCol{};
        int         gridRow{};
        glm::vec3   worldPos{};
        GameObject* gameObject{ nullptr };
    };

    class GameManagerComponent final : public BaseComponent, public Observer
    {
    public:
        GameManagerComponent(GameObject* owner, GameObject* pGridObject);
        ~GameManagerComponent() override;

        GameManagerComponent(const GameManagerComponent&)            = delete;
        GameManagerComponent(GameManagerComponent&&)                 = delete;
        GameManagerComponent& operator=(const GameManagerComponent&) = delete;
        GameManagerComponent& operator=(GameManagerComponent&&)      = delete;

        void FixedUpdate(float) override {}
        void Update()           override;
        void LateUpdate()       override; // nullifies dead entity pointers before scene frees them
        void Render()     const override {}

        void OnNotify(BaseComponent* entity, unsigned int eventID) override;
        void ChangeState(std::unique_ptr<GameState> newState);

        // Called by game states
        void SpawnPlayer(int col, int row);
        void SpawnPooka (int col, int row);
        void SpawnFygar (int col, int row);
        void ClearGameWorld();

        // Accessors
        GameObject* GetGridObject() const { return m_pGridObject; }
        GameObject* GetPlayer()     const { return m_PlayerSpawn.gameObject; }
        bool        IsGameOver()    const { return m_GameOver; }
        int         GetFinalScore() const { return m_FinalScore; }

    private:
        void SetupHUD();
        void RespawnEntities();
        void ResetToSpawn(SpawnInfo& spawn);
        void SetPlayerInvincible(bool invincible);

        GameObject* m_pGridObject{};

        std::unique_ptr<GameState> m_pCurrentState{};

        SpawnInfo              m_PlayerSpawn{};
        std::vector<SpawnInfo> m_EnemySpawns{};

        // HUD display objects (raw ptrs into the scene, NOT owned here)
        GameObject* m_pLivesDisplayObject{};
        GameObject* m_pScoreDisplayObject{};

        // Observers are owned by the manager – their lifetime is fully controlled here
        std::unique_ptr<LivesDisplayObserver> m_pLivesObserver{};
        std::unique_ptr<ScoreDisplayObserver> m_pScoreObserver{};

        bool  m_NeedsRespawn{ false };
        bool  m_GameOver    { false };
        int   m_FinalScore  { 0 };

        static constexpr float k_InvincibilityDuration{ 1.5f };
        float m_InvincibilityTimer{ 0.f };
        bool  m_IsInvincible{ false };
    };
}