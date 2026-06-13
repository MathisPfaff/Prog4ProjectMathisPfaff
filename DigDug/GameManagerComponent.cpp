#include "GameManagerComponent.h"
#include "GameState.h"
#include "MainMenuState.h"
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
#include <algorithm>

namespace dae
{
    // Grid layout constants (must match Main.cpp window sizing)
    static constexpr float kSidebarWidth = 155.f;
    static constexpr float kGridOffsetY  =  30.f;
    static constexpr int   kGridCols     =  14;
    static constexpr int   kGridRows     =  16;
    static constexpr float kCellSize     =  36.f;
    static constexpr float kGridWidth    = kGridCols * kCellSize;
    static constexpr float kGridHeight   = kGridRows * kCellSize;

    // ── DigLevel: one clean pre-dug layout used for all game modes ────────────
    static void DigLevel(GridComponent* grid)
    {
        // Pooka 1 tunnel – horizontal, row 2 (yellow zone), cols 1–4
        grid->PreDigCell(1, 2, TunnelSide::Right);
        grid->PreDigCell(2, 2, TunnelSide::Left | TunnelSide::Right);
        grid->PreDigCell(3, 2, TunnelSide::Left | TunnelSide::Right);
        grid->PreDigCell(4, 2, TunnelSide::Left);

        // Pooka 2 tunnel – horizontal, row 6 (orange zone), cols 9–12
        grid->PreDigCell(9,  6, TunnelSide::Right);
        grid->PreDigCell(10, 6, TunnelSide::Left | TunnelSide::Right);
        grid->PreDigCell(11, 6, TunnelSide::Left | TunnelSide::Right);
        grid->PreDigCell(12, 6, TunnelSide::Left);

        // Pooka 3 tunnel – vertical, col 2 (dark-orange zone), rows 9–12
        grid->PreDigCell(2,  9, TunnelSide::Down);
        grid->PreDigCell(2, 10, TunnelSide::Up | TunnelSide::Down);
        grid->PreDigCell(2, 11, TunnelSide::Up | TunnelSide::Down);
        grid->PreDigCell(2, 12, TunnelSide::Up);

        // Fygar tunnel – horizontal, row 13 (dark-red zone), cols 5–8
        grid->PreDigCell(5, 13, TunnelSide::Right);
        grid->PreDigCell(6, 13, TunnelSide::Left | TunnelSide::Right);
        grid->PreDigCell(7, 13, TunnelSide::Left | TunnelSide::Right);
        grid->PreDigCell(8, 13, TunnelSide::Left);

        // Player start tunnel – horizontal, row 7 (center), cols 5–8
        grid->PreDigCell(5, 7, TunnelSide::Right);
        grid->PreDigCell(6, 7, TunnelSide::Left | TunnelSide::Right);
        grid->PreDigCell(7, 7, TunnelSide::Left | TunnelSide::Right);
        grid->PreDigCell(8, 7, TunnelSide::Left);
    }

    GameManagerComponent::GameManagerComponent(GameObject* owner)
        : BaseComponent(owner)
    {
        ChangeState(std::make_unique<MainMenuState>());
    }

    GameManagerComponent::~GameManagerComponent()
    {
        m_pCurrentState.reset();
    }

    // ── SpawnGrid ─────────────────────────────────────────────────────────────

    void GameManagerComponent::SpawnGrid()
    {
        // Reset game-state flags for a fresh round
        m_GameOver       = false;
        m_PlayerWon      = false;
        m_NeedsRespawn   = false;
        m_IsInvincible   = false;
        m_InvincibilityTimer = 0.f;
        m_FinalScore     = 0;

        auto& scene = SceneManager::GetInstance().GetActiveScene();

        auto gridObj = std::make_unique<GameObject>();
        gridObj->SetLocalPosition(kSidebarWidth, kGridOffsetY);
        gridObj->AddComponent<GridComponent>(kGridWidth, kGridHeight, kGridCols, kGridRows);

        auto* grid = gridObj->GetComponent<GridComponent>();
        DigLevel(grid);   // ← replaces DigShapeA / DigShapeB / DigShapeC

        m_pGridObject = gridObj.get();
        scene.Add(std::move(gridObj));
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
        for (auto& spawn : m_EnemySpawns)
            if (spawn.gameObject && spawn.gameObject->IsMarkedForDestroy())
                spawn.gameObject = nullptr;

        if (m_PlayerSpawn.gameObject  && m_PlayerSpawn.gameObject->IsMarkedForDestroy())
            m_PlayerSpawn.gameObject  = nullptr;

        if (m_Player2Spawn.gameObject && m_Player2Spawn.gameObject->IsMarkedForDestroy())
            m_Player2Spawn.gameObject = nullptr;

        // Win condition: all enemies dead
        if (!m_PlayerWon && !m_GameOver && !m_EnemySpawns.empty())
        {
            const bool allDead = std::all_of(m_EnemySpawns.begin(), m_EnemySpawns.end(),
                [](const SpawnInfo& s) { return s.gameObject == nullptr; });

            if (allDead)
            {
                m_FinalScore = 0;
                if (m_PlayerSpawn.gameObject)
                    if (auto* sc = m_PlayerSpawn.gameObject->GetComponent<ScoreComponent>())
                        m_FinalScore += sc->GetScore();

                // Add P2 score when in two-player mode
                if (m_Player2Spawn.gameObject)
                    if (auto* sc = m_Player2Spawn.gameObject->GetComponent<ScoreComponent>())
                        m_FinalScore += sc->GetScore();

                m_PlayerWon = true;
            }
        }
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
            m_FinalScore = 0;
            if (m_PlayerSpawn.gameObject)
                if (auto* sc = m_PlayerSpawn.gameObject->GetComponent<ScoreComponent>())
                    m_FinalScore += sc->GetScore();

            // Add P2 score when in two-player mode
            if (m_Player2Spawn.gameObject)
                if (auto* sc = m_Player2Spawn.gameObject->GetComponent<ScoreComponent>())
                    m_FinalScore += sc->GetScore();

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
        player->AddComponent<HitboxComponent>(34.f, 34.f, HitboxType::Player);
        player->AddComponent<PlayerMovementComponent>(m_pGridObject);
        player->AddComponent<PumpComponent>(m_pGridObject);

        m_PlayerSpawn = { col, row, spawnWorld, player.get() };

        scene.Add(std::move(player));

        // Build the HUD now that the player's components exist
        SetupHUD();
    }

    // ── SetupHUD ──────────────────────────────────────────────────────────────

    void GameManagerComponent::SetupHUD()
    {
        auto* player = m_PlayerSpawn.gameObject;
        if (!player) return;

        auto* health = player->GetComponent<HealthComponent>();
        auto* score = player->GetComponent<ScoreComponent>();

        auto& scene = SceneManager::GetInstance().GetActiveScene();
        auto  font = ResourceManager::GetInstance().LoadFont("Lingua.otf", 14);

        auto livesObj = std::make_unique<GameObject>();
        livesObj->SetLocalPosition(10.f, 50.f);
        auto* livesText = livesObj->AddComponent<TextComponent>("Lives: 4", font, SDL_Color{ 255, 255, 255, 255 });
        m_pLivesDisplayObject = livesObj.get();
        scene.Add(std::move(livesObj));
        m_pLivesObserver = std::make_unique<LivesDisplayObserver>(health, livesText);

        auto scoreObj = std::make_unique<GameObject>();
        scoreObj->SetLocalPosition(10.f, 90.f);
        auto* scoreText = scoreObj->AddComponent<TextComponent>("Score: 0", font, SDL_Color{ 255, 255, 255, 255 });
        m_pScoreDisplayObject = scoreObj.get();
        scene.Add(std::move(scoreObj));
        m_pScoreObserver = std::make_unique<ScoreDisplayObserver>(score, scoreText);
    }

    // ── SpawnPooka ────────────────────────────────────────────────────────────

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
        pooka->AddComponent<HitboxComponent>(34.f, 34.f, HitboxType::Enemy);
        pooka->AddComponent<PookaComponent>(m_pGridObject);

        m_EnemySpawns.push_back({ col, row, spawnWorld, pooka.get() });
        scene.Add(std::move(pooka));
    }

    // ── SpawnFygar ────────────────────────────────────────────────────────────

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
        fygar->AddComponent<HitboxComponent>(34.f, 34.f, HitboxType::Enemy);
        fygar->AddComponent<FireBreathComponent>(m_pGridObject);
        fygar->AddComponent<FygarComponent>(m_pGridObject);

        m_EnemySpawns.push_back({ col, row, spawnWorld, fygar.get() });
        scene.Add(std::move(fygar));
    }

    void GameManagerComponent::SpawnPlayer2(int col, int row)
    {
        auto* grid = m_pGridObject->GetComponent<GridComponent>();
        if (!grid) return;

        auto& scene = SceneManager::GetInstance().GetActiveScene();

        float wx{}, wy{};
        grid->CellToWorld(col, row, wx, wy);
        const glm::vec3 spawnWorld = m_pGridObject->GetWorldPosition() + glm::vec3(wx, wy, 0.f);

        auto player2 = std::make_unique<GameObject>();
        player2->SetLocalPosition(spawnWorld);
        player2->AddComponent<TextureComponent>("Player2.png", 2.f);

        auto* health = player2->AddComponent<HealthComponent>(4);
        health->AddObserver(this);   // ← was missing: enemies never triggered a respawn

        player2->AddComponent<ScoreComponent>();
        player2->AddComponent<HitboxComponent>(34.f, 34.f, HitboxType::Player);
        player2->AddComponent<PlayerMovementComponent>(m_pGridObject);
        player2->AddComponent<PumpComponent>(m_pGridObject);

        m_Player2Spawn = { col, row, spawnWorld, player2.get() };

        scene.Add(std::move(player2));

        SetupHUD2();
    }

    // ── SetupHUD2 ─────────────────────────────────────────────────────────────

    void GameManagerComponent::SetupHUD2()
    {
        auto* player2 = m_Player2Spawn.gameObject;
        if (!player2) return;

        auto* health = player2->GetComponent<HealthComponent>();
        auto* score  = player2->GetComponent<ScoreComponent>();

        auto& scene = SceneManager::GetInstance().GetActiveScene();
        auto  font  = ResourceManager::GetInstance().LoadFont("Lingua.otf", 14);

        // P2 lives – lower in the sidebar
        auto livesObj = std::make_unique<GameObject>();
        livesObj->SetLocalPosition(10.f, 470.f);
        auto* livesText = livesObj->AddComponent<TextComponent>("P2 Lives: 4", font, SDL_Color{ 100, 200, 255, 255 });
        m_pLives2DisplayObject = livesObj.get();
        scene.Add(std::move(livesObj));
        m_pLives2Observer = std::make_unique<LivesDisplayObserver>(health, livesText);

        // P2 score
        auto scoreObj = std::make_unique<GameObject>();
        scoreObj->SetLocalPosition(10.f, 510.f);
        auto* scoreText = scoreObj->AddComponent<TextComponent>("P2 Score: 0", font, SDL_Color{ 100, 200, 255, 255 });
        m_pScore2DisplayObject = scoreObj.get();
        scene.Add(std::move(scoreObj));
        m_pScore2Observer = std::make_unique<ScoreDisplayObserver>(score, scoreText);
    }

    // ── Clear game world ──────────────────────────────────────────────────────

    void GameManagerComponent::ClearGameWorld()
    {
        // Unregister observers while components are still alive
        if (m_pLivesObserver)  m_pLivesObserver->ClearReferences();
        if (m_pScoreObserver)  m_pScoreObserver->ClearReferences();
        if (m_pLives2Observer) m_pLives2Observer->ClearReferences();
        if (m_pScore2Observer) m_pScore2Observer->ClearReferences();

        m_pLivesObserver.reset();
        m_pScoreObserver.reset();
        m_pLives2Observer.reset();
        m_pScore2Observer.reset();

        // Player 1
        if (m_PlayerSpawn.gameObject && !m_PlayerSpawn.gameObject->IsMarkedForDestroy())
            m_PlayerSpawn.gameObject->MarkForDestroy();
        m_PlayerSpawn.gameObject = nullptr;

        // Player 2
        if (m_Player2Spawn.gameObject && !m_Player2Spawn.gameObject->IsMarkedForDestroy())
            m_Player2Spawn.gameObject->MarkForDestroy();
        m_Player2Spawn.gameObject = nullptr;

        // Enemies
        for (auto& spawn : m_EnemySpawns)
            if (spawn.gameObject && !spawn.gameObject->IsMarkedForDestroy())
                spawn.gameObject->MarkForDestroy();
        m_EnemySpawns.clear();

        // HUD P1
        if (m_pLivesDisplayObject && !m_pLivesDisplayObject->IsMarkedForDestroy())
            m_pLivesDisplayObject->MarkForDestroy();
        m_pLivesDisplayObject = nullptr;

        if (m_pScoreDisplayObject && !m_pScoreDisplayObject->IsMarkedForDestroy())
            m_pScoreDisplayObject->MarkForDestroy();
        m_pScoreDisplayObject = nullptr;

        // HUD P2
        if (m_pLives2DisplayObject && !m_pLives2DisplayObject->IsMarkedForDestroy())
            m_pLives2DisplayObject->MarkForDestroy();
        m_pLives2DisplayObject = nullptr;

        if (m_pScore2DisplayObject && !m_pScore2DisplayObject->IsMarkedForDestroy())
            m_pScore2DisplayObject->MarkForDestroy();
        m_pScore2DisplayObject = nullptr;

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

        // ← was missing: Player 2 was never reset after a hit
        if (m_Player2Spawn.gameObject &&
            !m_Player2Spawn.gameObject->IsMarkedForDestroy())
        {
            ResetToSpawn(m_Player2Spawn);

            if (auto* mv = m_Player2Spawn.gameObject->GetComponent<PlayerMovementComponent>())
                mv->Reset();
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