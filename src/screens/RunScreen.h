#pragma once

#include "assets/AssetManager.h"
#include "ecs/Components.h"
#include "ecs/World.h"
#include "game/Perks.h"
#include "game/PersistentState.h"
#include "screens/Screen.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <random>
#include <string>

enum class DefeatReason : std::uint8_t
{
    None,
    Hp,
    Arousal
};

class RunScreen final : public Screen
{
public:
    RunScreen(const sf::Font& uiFont, bool hasUiFont, PersistentState& persistentState);

    std::optional<GameState> handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    struct PlayerRuntimeStats
    {
        float damageMultiplier{1.0f};
        float attackIntervalMultiplier{1.0f};
        float speedMultiplier{1.0f};
        float attackRangeMultiplier{1.0f};
    };

    void createPlayerEntity();
    void spawnMeleeEnemy(float x, float y);
    void spawnRangedEnemy(float x, float y);
    void spawnCasterEnemy(float x, float y);
    void spawnResourcePickup(float x, float y, int amount);
    void commitRunResultIfNeeded(LastRunOutcome outcome);
    void updateWaveSystem();
    bool tryFinishRun();
    void updatePlayerMovement(float deltaTime);
    void updateEnemyAI(float deltaTime);
    void applyContactDamage(float deltaTime);
    void updateAutoAttack(float deltaTime);
    void updateRangedAttacks(float deltaTime);
    void spawnEnemyProjectile(float originX,
                              float originY,
                              float dirX,
                              float dirY,
                              float speed,
                              float damage,
                              ProjectileVisual visual,
                              float arousalDamage);
    void applyArousalDamage(float amount);
    void updateProjectiles(float deltaTime);
    void handleProjectileCollisions();
    void updateLifetimes(float deltaTime);
    void updatePickups(float deltaTime);
    void handleDeaths();
    void handleLevelUpProgression();
    int computeXpToNext(int level) const;
    void spawnXpPickup(float x, float y, int xpValue);
    EntityId findNearestEnemyInRange(float range);
    void applyDamageToEntity(EntityId entity, float damage);
    std::size_t countAliveEnemies();
    std::size_t countProjectiles();
    void rollLevelUpChoices();
    void applyPerk(PerkId perkId);
    void updateHudText();
    void renderEntities(sf::RenderWindow& window);
    void renderLevelUpOverlay(sf::RenderWindow& window);
    void renderStopOverlay(sf::RenderWindow& window);
    void renderResultOverlay(sf::RenderWindow& window);

    static constexpr std::size_t WaveCount = 3;

    bool m_hasUiFont{false};
    bool m_isDefeated{false};
    DefeatReason m_defeatReason{DefeatReason::None};
    bool m_isLevelUpSelection{false};
    bool m_isStopped{false};
    bool m_isShowingResult{false};
    bool m_resultApplied{false};
    int m_pendingLevelUps{0};
    int m_runResourceRaw{0};
    float m_incomingHpMultiplier{1.0f};
    float m_incomingArousalMultiplier{1.0f};
    float m_hitFeedbackTimer{0.0f};
    float m_runDuration{180.0f};
    float m_runTimeLeft{180.0f};
    std::array<bool, WaveCount> m_wavesSpawned{};
    PersistentState& m_persistentState;
    AssetManager m_assetManager;
    World m_world;
    EntityId m_playerEntity{InvalidEntity};
    std::mt19937 m_rng;
    std::array<std::string, 3> m_levelChoices{};
    std::array<PerkId, 3> m_levelChoiceIds{
        PerkId::PowerI,
        PerkId::AttackSpeedI,
        PerkId::VitalityI
    };
    std::string m_lastSelectedPerk;
    PlayerRuntimeStats m_runtimeStats;
    sf::Text m_title;
    sf::Text m_instruction;
    sf::Text m_hudText;
    sf::Text m_feedbackText;
    sf::Text m_levelUpTitleText;
    sf::Text m_levelUpChoicesText;
    sf::Text m_lastPerkText;
    sf::Text m_stopTitleText;
    sf::Text m_stopChoicesText;
    sf::Text m_resultTitleText;
    sf::Text m_resultBodyText;
    sf::Text m_resultHintText;
};
