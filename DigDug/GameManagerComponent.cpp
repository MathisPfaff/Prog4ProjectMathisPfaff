#include "GameManagerComponent.h"
#include "GameState.h"
#include "PlayingState.h"
#include "Scene.h"
#include "GameObject.h"
#include "GridComponent.h"
#include "TextureComponent.h"
#include "TextComponent.h"
#include "ResourceManager.h"
#include "HealthComponent.h"
#include "HitboxComponent.h"
#include "PlayerMovementComponent.h"
#include "PumpComponent.h"
#include "PookaComponent.h"
#include "FygarComponent.h"
#include "FireBreathComponent.h"
#include "ScoreComponent.h"
#include "SceneManager.h"
#include "GameTime.h"
#include "Hash.h"

namespace dae
{
    GameManagerComponent::GameManagerComponent(GameObject* owner, GameObject* pGridObject)
        : BaseComponent(owner)
        , m_pGridObject(pGridObject)
    {
        ChangeState(std::make_unique<PlayingState>());
    }

    GameManagerComponent::~GameManagerComponent()
    {
        m_pCurrentState.reset();
    }

    // ── Update ────────────────────────────────────────────────────────────────

    void GameManagerComponent::Update()
    {
        if (m_IsInvincible)
        {
            m_InvincibilityTimer -= GameTime::GetInstance().GetDeltaTime();
            if (m_InvincibilityTimer <= 0.f)
            {
                m_InvincibilityTimer = 0.f;
                SetPlayerInvincible(false);
            }
        }

        if (m_NeedsRespawn)
        {
            RespawnEntities();
            m_NeedsRespawn = false;
        }

        if (m_pCurrentState)
        {
            auto nextState = m_pCurrentState->Update(this);
            if (nextState)
                ChangeState(std::move(nextState));
        }
    }

    // ── LateUpdate ────────────────────────────────────────────────────────────
    // Runs BEFORE Scene::LateUpdate() frees destroyed objects.
    // Nullify any raw pointers that point to objects marked for destroy this frame
    // so they are never dereferenced after the memory is freed.

    void GameManagerComponent::LateUpdate()
    {
        // Enemies that were pumped to death this frame
        for (auto& spawn : m_EnemySpawns)
        {
            if (spawn.gameObject && spawn.gameObject->IsMarkedForDestroy())
                spawn.gameObject = nullptr;
        }

        // Player (should not normally happen mid-game, but guard anyway)
        if (m_PlayerSpawn.gameObject && m_PlayerSpawn.gameObject->IsMarkedForDestroy())
            m_PlayerSpawn.gameObject = nullptr;
    }

    // ── Observer ──────────────────────────────────────────────────────────────

    void GameManagerComponent::OnNotify(BaseComponent* entity, unsigned int eventID)
    {
        if (m_IsInvincible) return;

        if (eventID == HealthEvent::Changed)
        {
            if (auto* health = dynamic_cast<HealthComponent*>(entity))
            {
                if (!health->IsDead())
                    m_NeedsRespawn = true;
            }
        }
        else if (eventID == HealthEvent::Died)
        {
            if (m_PlayerSpawn.gameObject)
            {
                if (auto* sc = m_PlayerSpawn.gameObject->GetComponent<ScoreComponent>())
                    m_FinalScore = sc->GetScore();
            }
            m_GameOver = true;
        }
    }

    // ── State machine ─────────────────────────────────────────────────────────

    void GameManagerComponent::ChangeState(std::unique_ptr<GameState> newState)
    {
        if (m_pCurrentState)
            m_pCurrentState->OnExit(this);

        m_pCurrentState = std::move(newState);

        if (m_pCurrentState)
            m_pCurrentState->OnEnter(this);
    }

    // ── Entity spawning ───────────────────────────────────────────────────────

    void GameManagerComponent::SpawnPlayer(int col, int row)
    {
        auto* grid = m_pGridObject->GetComponent<GridComponent>();
        if (!grid) return;

        auto& scene = SceneManager::GetInstance().GetActiveScene();

        float wx{}, wy{};
        grid->CellToWorld(col, row, wx, wy);
        const glm::vec3 spawnWorld = m_pGridObject->GetWorldPosition() + glm::vec3(wx, wy, 0.f);

        auto player = std::make_unique<GameObject>();
        player->SetLocalPosition(spawnWorld);
        player->AddComponent<TextureComponent>("Player.png", 2.f);

        auto* health = player->AddComponent<HealthComponent>(4);
        health->AddObserver(this);

        player->AddComponent<ScoreComponent>();
        player->AddComponent<HitboxComponent>(36.f, 36.f, HitboxType::Player);
        player->AddComponent<PlayerMovementComponent>(m_pGridObject);
        player->AddComponent<PumpComponent>(m_pGridObject);

        m_PlayerSpawn = { col, row, spawnWorld, player.get() };

        scene.Add(std::move(player));

        // Build the HUD now that the player's components exist
        SetupHUD();
    }

    void GameManagerComponent::SpawnPooka(int col, int row)
    {
        auto* grid = m_pGridObject->GetComponent<GridComponent>();
        if (!grid) return;

        auto& scene = SceneManager::GetInstance().GetActiveScene();

        float wx{}, wy{};
        grid->CellToWorld(col, row, wx, wy);
        const glm::vec3 spawnWorld = m_pGridObject->GetWorldPosition() + glm::vec3(wx, wy, 0.f);

        auto pooka = std::make_unique<GameObject>();
        pooka->SetLocalPosition(spawnWorld);
        pooka->AddComponent<TextureComponent>("Pooka.png", 2.f);
        pooka->AddComponent<HitboxComponent>(36.f, 36.f, HitboxType::Enemy);
        pooka->AddComponent<PookaComponent>(m_pGridObject);

        m_EnemySpawns.push_back({ col, row, spawnWorld, pooka.get() });
        scene.Add(std::move(pooka));
    }

    void GameManagerComponent::SpawnFygar(int col, int row)
    {
        auto* grid = m_pGridObject->GetComponent<GridComponent>();
        if (!grid) return;

        auto& scene = SceneManager::GetInstance().GetActiveScene();

        float wx{}, wy{};
        grid->CellToWorld(col, row, wx, wy);
        const glm::vec3 spawnWorld = m_pGridObject->GetWorldPosition() + glm::vec3(wx, wy, 0.f);

        auto fygar = std::make_unique<GameObject>();
        fygar->SetLocalPosition(spawnWorld);
        fygar->AddComponent<TextureComponent>("Fygar.png", 2.f);
        fygar->AddComponent<HitboxComponent>(36.f, 36.f, HitboxType::Enemy);
        fygar->AddComponent<FireBreathComponent>(m_pGridObject);
        fygar->AddComponent<FygarComponent>(m_pGridObject);

        m_EnemySpawns.push_back({ col, row, spawnWorld, fygar.get() });
        scene.Add(std::move(fygar));
    }

    // ── HUD setup ─────────────────────────────────────────────────────────────

    void GameManagerComponent::SetupHUD()
    {
        auto* player = m_PlayerSpawn.gameObject;
        if (!player) return;

        auto* health = player->GetComponent<HealthComponent>();
        auto* score  = player->GetComponent<ScoreComponent>();

        auto& scene = SceneManager::GetInstance().GetActiveScene();
        auto  font  = ResourceManager::GetInstance().LoadFont("Lingua.otf", 14);

        // Lives display
        auto livesObj  = std::make_unique<GameObject>();
        livesObj->SetLocalPosition(10.f, 50.f);
        auto* livesText = livesObj->AddComponent<TextComponent>("Lives: 4", font, SDL_Color{ 255, 255, 255, 255 });
        m_pLivesDisplayObject = livesObj.get();
        scene.Add(std::move(livesObj));
        m_pLivesObserver = std::make_unique<LivesDisplayObserver>(health, livesText);

        // Score display
        auto scoreObj  = std::make_unique<GameObject>();
        scoreObj->SetLocalPosition(10.f, 90.f);
        auto* scoreText = scoreObj->AddComponent<TextComponent>("Score: 0", font, SDL_Color{ 255, 255, 255, 255 });
        m_pScoreDisplayObject = scoreObj.get();
        scene.Add(std::move(scoreObj));
        m_pScoreObserver = std::make_unique<ScoreDisplayObserver>(score, scoreText);
    }

    // ── Clear game world ──────────────────────────────────────────────────────

    void GameManagerComponent::ClearGameWorld()
    {
        // Unregister observers from their subjects NOW, while HealthComponent and
        // ScoreComponent are still alive (player is only marked for destroy here,
        // actual destruction happens on the next LateUpdate).
        if (m_pLivesObserver) m_pLivesObserver->ClearReferences();
        if (m_pScoreObserver) m_pScoreObserver->ClearReferences();

        // Now destroy the observers – their destructors are empty so no double-unregister.
        m_pLivesObserver.reset();
        m_pScoreObserver.reset();

        // Player
        if (m_PlayerSpawn.gameObject && !m_PlayerSpawn.gameObject->IsMarkedForDestroy())
            m_PlayerSpawn.gameObject->MarkForDestroy();
        m_PlayerSpawn.gameObject = nullptr;

        // Enemies
        for (auto& spawn : m_EnemySpawns)
        {
            if (spawn.gameObject && !spawn.gameObject->IsMarkedForDestroy())
                spawn.gameObject->MarkForDestroy();
        }
        m_EnemySpawns.clear();

        // HUD
        if (m_pLivesDisplayObject && !m_pLivesDisplayObject->IsMarkedForDestroy())
            m_pLivesDisplayObject->MarkForDestroy();
        m_pLivesDisplayObject = nullptr;

        if (m_pScoreDisplayObject && !m_pScoreDisplayObject->IsMarkedForDestroy())
            m_pScoreDisplayObject->MarkForDestroy();
        m_pScoreDisplayObject = nullptr;

        // Grid
        if (m_pGridObject && !m_pGridObject->IsMarkedForDestroy())
            m_pGridObject->MarkForDestroy();
        m_pGridObject = nullptr;
    }

    // ── Respawn helpers ───────────────────────────────────────────────────────

    void GameManagerComponent::RespawnEntities()
    {
        if (m_PlayerSpawn.gameObject &&
            !m_PlayerSpawn.gameObject->IsMarkedForDestroy())
        {
            ResetToSpawn(m_PlayerSpawn);

            if (auto* mv = m_PlayerSpawn.gameObject->GetComponent<PlayerMovementComponent>())
                mv->Reset();

            SetPlayerInvincible(true);
            m_InvincibilityTimer = k_InvincibilityDuration;
        }

        for (auto& spawn : m_EnemySpawns)
        {
            // nullptr means killed by pump – skip, it's gone
            if (spawn.gameObject && !spawn.gameObject->IsMarkedForDestroy())
                ResetToSpawn(spawn);
        }
    }

    void GameManagerComponent::ResetToSpawn(SpawnInfo& spawn)
    {
        spawn.gameObject->SetLocalPosition(spawn.worldPos);

        if (auto* pooka = spawn.gameObject->GetComponent<PookaComponent>())
            pooka->ResetToWalking();

        if (auto* fygar = spawn.gameObject->GetComponent<FygarComponent>())
            fygar->ResetToWalking();
    }

    void GameManagerComponent::SetPlayerInvincible(bool invincible)
    {
        m_IsInvincible = invincible;

        if (!m_PlayerSpawn.gameObject) return;

        if (auto* hb = m_PlayerSpawn.gameObject->GetComponent<HitboxComponent>())
            hb->SetEnabled(!invincible);
    }
}