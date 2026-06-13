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
        explicit GameManagerComponent(GameObject* owner);
        ~GameManagerComponent() override;

        GameManagerComponent(const GameManagerComponent&)            = delete;
        GameManagerComponent(GameManagerComponent&&)                 = delete;
        GameManagerComponent& operator=(const GameManagerComponent&) = delete;
        GameManagerComponent& operator=(GameManagerComponent&&)      = delete;

        void FixedUpdate(float) override {}
        void Update()           override;
        void LateUpdate()       override;
        void Render()     const override {}

        void OnNotify(BaseComponent* entity, unsigned int eventID) override;
        void ChangeState(std::unique_ptr<GameState> newState);

        void SpawnGrid();
        void SpawnPlayer (int col, int row);
        void SpawnPlayer2(int col, int row);
        void SpawnPooka  (int col, int row);
        void SpawnFygar  (int col, int row);
        void ClearGameWorld();

        void RequestStart()      { m_StartRequested = true; }
        bool IsStartRequested()  const { return m_StartRequested; }
        void ResetStartRequest() { m_StartRequested = false; }

        GameObject* GetGridObject() const { return m_pGridObject; }
        GameObject* GetPlayer()     const { return m_PlayerSpawn.gameObject; }
        GameObject* GetPlayer2()    const { return m_Player2Spawn.gameObject; }
        bool        IsGameOver()    const { return m_GameOver; }
        bool        IsPlayerWon()   const { return m_PlayerWon; }
        int         GetFinalScore() const { return m_FinalScore; }

    private:
        void SetupHUD();
        void SetupHUD2();
        void RespawnEntities();
        void ResetToSpawn(SpawnInfo& spawn);
        void SetPlayerInvincible(bool invincible);

        GameObject* m_pGridObject{};

        std::unique_ptr<GameState> m_pCurrentState{};

        SpawnInfo              m_PlayerSpawn{};
        SpawnInfo              m_Player2Spawn{};
        std::vector<SpawnInfo> m_EnemySpawns{};

        GameObject* m_pLivesDisplayObject{};
        GameObject* m_pScoreDisplayObject{};
        std::unique_ptr<LivesDisplayObserver> m_pLivesObserver{};
        std::unique_ptr<ScoreDisplayObserver> m_pScoreObserver{};

        GameObject* m_pLives2DisplayObject{};
        GameObject* m_pScore2DisplayObject{};
        std::unique_ptr<LivesDisplayObserver> m_pLives2Observer{};
        std::unique_ptr<ScoreDisplayObserver> m_pScore2Observer{};

        bool  m_NeedsRespawn  { false };
        bool  m_GameOver      { false };
        bool  m_PlayerWon     { false };
        bool  m_StartRequested{ false };
        int   m_FinalScore    { 0 };

        static constexpr float k_InvincibilityDuration{ 1.5f };
        float m_InvincibilityTimer{ 0.f };
        bool  m_IsInvincible{ false };
    };
}