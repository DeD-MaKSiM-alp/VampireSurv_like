#pragma once

#include "assets/AssetManager.h"
#include "ecs/Components.h"
#include "ecs/World.h"
#include "game/Perks.h"
#include "screens/Screen.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

#include <array>
#include <cstddef>
#include <random>
#include <string>

class RunScreen final : public Screen
{
public:
    RunScreen(const sf::Font& uiFont, bool hasUiFont);

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
    void spawnEnemies();
    void spawnMeleeEnemy(float x, float y);
    void spawnRangedEnemy(float x, float y);
    void spawnCasterEnemy(float x, float y);
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
                              ProjectileVisual visual);
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

    bool m_hasUiFont{false};
    bool m_isDefeated{false};
    bool m_isLevelUpSelection{false};
    int m_pendingLevelUps{0};
    float m_hitFeedbackTimer{0.0f};
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
};
