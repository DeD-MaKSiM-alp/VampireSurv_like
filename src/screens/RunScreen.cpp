#include "screens/RunScreen.h"

#include "ecs/Components.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <vector>

namespace
{
constexpr float WindowWidth = 1280.0f;
constexpr float WindowHeight = 720.0f;

constexpr float PlayerSpeed = 220.0f;
constexpr float PlayerWidth = 48.0f;
constexpr float PlayerHeight = 48.0f;
constexpr float PlayerColliderRadius = 18.0f;

constexpr float EnemyWidth = 40.0f;
constexpr float EnemyHeight = 40.0f;
constexpr float EnemyColliderRadius = 18.0f;
constexpr std::size_t EnemySpawnCount = 6;

constexpr float XpPickupWidth = 20.0f;
constexpr float XpPickupHeight = 20.0f;
constexpr float XpPickupColliderRadius = 12.0f;
constexpr float XpPickupLifetime = 10.0f;

constexpr int MeleeXpReward = 10;

const std::array<sf::Vector2f, 8> EnemySpawnPoints = {
    sf::Vector2f{80.0f, 80.0f},
    sf::Vector2f{560.0f, 80.0f},
    sf::Vector2f{1000.0f, 80.0f},
    sf::Vector2f{80.0f, 560.0f},
    sf::Vector2f{560.0f, 560.0f},
    sf::Vector2f{1000.0f, 560.0f},
    sf::Vector2f{80.0f, 320.0f},
    sf::Vector2f{1000.0f, 320.0f},
};

}

RunScreen::RunScreen(const sf::Font& uiFont, bool hasUiFont)
    : m_hasUiFont(hasUiFont)
    , m_rng(std::random_device{}())
{
    m_assetManager.loadTexture("player", "assets/textures/placeholders/player.png");
    m_assetManager.loadTexture("enemy_melee", "assets/textures/placeholders/enemy_melee.png");
    m_assetManager.loadTexture("xp_pickup", "assets/textures/placeholders/xp_pickup.png");

    createPlayerEntity();
    spawnMeleeEnemies();

    if (m_hasUiFont)
    {
        m_title.setFont(uiFont);
        m_title.setString("Run Stub");
        m_title.setCharacterSize(46);
        m_title.setFillColor(sf::Color(245, 230, 220));
        m_title.setPosition(32.f, 16.f);

        m_instruction.setFont(uiFont);
        m_instruction.setCharacterSize(22);
        m_instruction.setFillColor(sf::Color(255, 205, 170));
        m_instruction.setPosition(32.f, 64.f);

        m_hudText.setFont(uiFont);
        m_hudText.setCharacterSize(20);
        m_hudText.setFillColor(sf::Color(200, 235, 255));
        m_hudText.setPosition(32.f, 100.f);

        m_feedbackText.setFont(uiFont);
        m_feedbackText.setCharacterSize(20);
        m_feedbackText.setFillColor(sf::Color(255, 100, 100));
        m_feedbackText.setPosition(32.f, 160.f);
        m_feedbackText.setString("Hit!");

        m_levelUpTitleText.setFont(uiFont);
        m_levelUpTitleText.setCharacterSize(34);
        m_levelUpTitleText.setFillColor(sf::Color(255, 235, 170));
        m_levelUpTitleText.setPosition(320.f, 220.f);

        m_levelUpChoicesText.setFont(uiFont);
        m_levelUpChoicesText.setCharacterSize(26);
        m_levelUpChoicesText.setFillColor(sf::Color(230, 230, 255));
        m_levelUpChoicesText.setPosition(330.f, 280.f);

        m_lastPerkText.setFont(uiFont);
        m_lastPerkText.setCharacterSize(18);
        m_lastPerkText.setFillColor(sf::Color(190, 230, 190));
        m_lastPerkText.setPosition(32.f, 188.f);
    }

    updateHudText();
}

std::optional<GameState> RunScreen::handleEvent(const sf::Event& event)
{
    if (event.type != sf::Event::KeyPressed)
    {
        return std::nullopt;
    }

    if (m_isDefeated && event.key.code == sf::Keyboard::Enter)
    {
        return GameState::Base;
    }

    if (!m_isDefeated && event.key.code == sf::Keyboard::Escape)
    {
        return GameState::Base;
    }

    if (m_isLevelUpSelection && !m_isDefeated)
    {
        int choiceIndex = -1;
        if (event.key.code == sf::Keyboard::Num1 || event.key.code == sf::Keyboard::Numpad1)
        {
            choiceIndex = 0;
        }
        else if (event.key.code == sf::Keyboard::Num2 || event.key.code == sf::Keyboard::Numpad2)
        {
            choiceIndex = 1;
        }
        else if (event.key.code == sf::Keyboard::Num3 || event.key.code == sf::Keyboard::Numpad3)
        {
            choiceIndex = 2;
        }

        if (choiceIndex >= 0)
        {
            const std::size_t picked = static_cast<std::size_t>(choiceIndex);
            applyPerk(m_levelChoiceIds[picked]);

            if (m_pendingLevelUps > 0)
            {
                --m_pendingLevelUps;
            }

            if (m_pendingLevelUps > 0)
            {
                rollLevelUpChoices();
            }
            else
            {
                m_isLevelUpSelection = false;
            }
        }
    }

    return std::nullopt;
}

void RunScreen::update(float deltaTime)
{
    m_hitFeedbackTimer = std::max(0.0f, m_hitFeedbackTimer - deltaTime);

    if (!m_isDefeated && !m_isLevelUpSelection)
    {
        updatePlayerMovement(deltaTime);
        updateEnemies(deltaTime);
        applyContactDamage(deltaTime);
        updateAutoAttack(deltaTime);
        updatePickups(deltaTime);
        handleDeaths();
        handleLevelUpProgression();
    }
    else if (!m_isDefeated && m_isLevelUpSelection)
    {
        // Pause gameplay simulation while the player selects a perk.
    }

    m_world.flushDestroyed();

    if (!m_isDefeated)
    {
        if (const auto* health = m_world.getComponent<HealthComponent>(m_playerEntity))
        {
            if (health->currentHp <= 0.0f)
            {
                m_isDefeated = true;
                m_isLevelUpSelection = false;
                m_pendingLevelUps = 0;
            }
        }
    }

    updateHudText();
}

void RunScreen::render(sf::RenderWindow& window)
{
    window.clear(sf::Color(32, 22, 20));
    renderEntities(window);

    if (m_hasUiFont)
    {
        window.draw(m_title);
        window.draw(m_instruction);
        window.draw(m_hudText);
        window.draw(m_lastPerkText);

        if (m_hitFeedbackTimer > 0.0f)
        {
            window.draw(m_feedbackText);
        }

        if (m_isLevelUpSelection)
        {
            renderLevelUpOverlay(window);
        }
    }
}

void RunScreen::createPlayerEntity()
{
    m_playerEntity = m_world.createEntity();

    m_world.addComponent<PlayerTagComponent>(m_playerEntity);
    m_world.addComponent<TransformComponent>(m_playerEntity, TransformComponent{
        (WindowWidth - PlayerWidth) * 0.5f,
        (WindowHeight - PlayerHeight) * 0.5f
    });
    m_world.addComponent<VelocityComponent>(m_playerEntity, VelocityComponent{0.0f, 0.0f});
    m_world.addComponent<SpriteComponent>(m_playerEntity, SpriteComponent{
        "player",
        PlayerWidth,
        PlayerHeight,
        255,
        255,
        255,
        255
    });
    m_world.addComponent<ColliderComponent>(m_playerEntity, ColliderComponent{PlayerColliderRadius});
    m_world.addComponent<HealthComponent>(m_playerEntity, HealthComponent{100.0f, 100.0f});
    m_world.addComponent<ArousalComponent>(m_playerEntity, ArousalComponent{0.0f, 100.0f});
    m_world.addComponent<AutoAttackComponent>(m_playerEntity, AutoAttackComponent{16.0f, 260.0f, 0.55f, 0.0f});
    m_world.addComponent<ExperienceComponent>(m_playerEntity, ExperienceComponent{1, 0, computeXpToNext(1)});
}

void RunScreen::spawnMeleeEnemies()
{
    for (std::size_t i = 0; i < EnemySpawnCount && i < EnemySpawnPoints.size(); ++i)
    {
        const EntityId enemy = m_world.createEntity();

        m_world.addComponent<EnemyComponent>(enemy, EnemyComponent{EnemyArchetype::Melee, 140.0f});
        m_world.addComponent<TransformComponent>(enemy, TransformComponent{
            EnemySpawnPoints[i].x,
            EnemySpawnPoints[i].y
        });
        m_world.addComponent<VelocityComponent>(enemy, VelocityComponent{0.0f, 0.0f});
        m_world.addComponent<SpriteComponent>(enemy, SpriteComponent{
            "enemy_melee",
            EnemyWidth,
            EnemyHeight,
            255,
            170,
            170,
            255
        });
        m_world.addComponent<ColliderComponent>(enemy, ColliderComponent{EnemyColliderRadius});
        m_world.addComponent<HealthComponent>(enemy, HealthComponent{40.0f, 40.0f});
        m_world.addComponent<ContactDamageComponent>(enemy, ContactDamageComponent{8.0f, 0.9f, 0.0f});
    }
}

void RunScreen::updatePlayerMovement(float deltaTime)
{
    if (!m_world.isAlive(m_playerEntity))
    {
        return;
    }

    auto* transform = m_world.getComponent<TransformComponent>(m_playerEntity);
    auto* velocity = m_world.getComponent<VelocityComponent>(m_playerEntity);
    auto* sprite = m_world.getComponent<SpriteComponent>(m_playerEntity);

    if (!transform || !velocity || !sprite)
    {
        return;
    }

    float directionX = 0.0f;
    float directionY = 0.0f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
        directionX -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
        directionX += 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    {
        directionY -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
        directionY += 1.0f;
    }

    const float lengthSquared = directionX * directionX + directionY * directionY;
    if (lengthSquared > 0.0f)
    {
        const float invLength = 1.0f / std::sqrt(lengthSquared);
        directionX *= invLength;
        directionY *= invLength;
    }

    const float effectiveSpeed = PlayerSpeed * m_runtimeStats.speedMultiplier;
    velocity->vx = directionX * effectiveSpeed;
    velocity->vy = directionY * effectiveSpeed;

    transform->x += velocity->vx * deltaTime;
    transform->y += velocity->vy * deltaTime;

    transform->x = std::clamp(transform->x, 0.0f, WindowWidth - sprite->width);
    transform->y = std::clamp(transform->y, 0.0f, WindowHeight - sprite->height);
}

void RunScreen::updateEnemies(float deltaTime)
{
    auto* playerTransform = m_world.getComponent<TransformComponent>(m_playerEntity);
    auto* playerSprite = m_world.getComponent<SpriteComponent>(m_playerEntity);

    if (!playerTransform || !playerSprite)
    {
        return;
    }

    const float playerCenterX = playerTransform->x + playerSprite->width * 0.5f;
    const float playerCenterY = playerTransform->y + playerSprite->height * 0.5f;

    m_world.forEach<EnemyComponent, TransformComponent, VelocityComponent, SpriteComponent>(
        [&](EntityId, EnemyComponent& enemy, TransformComponent& transform, VelocityComponent& velocity, SpriteComponent& sprite)
        {
            float directionX = playerCenterX - (transform.x + sprite.width * 0.5f);
            float directionY = playerCenterY - (transform.y + sprite.height * 0.5f);
            const float lengthSquared = directionX * directionX + directionY * directionY;

            if (lengthSquared > 0.001f)
            {
                const float invLength = 1.0f / std::sqrt(lengthSquared);
                directionX *= invLength;
                directionY *= invLength;
            }
            else
            {
                directionX = 0.0f;
                directionY = 0.0f;
            }

            velocity.vx = directionX * enemy.moveSpeed;
            velocity.vy = directionY * enemy.moveSpeed;
            transform.x += velocity.vx * deltaTime;
            transform.y += velocity.vy * deltaTime;

            transform.x = std::clamp(transform.x, 0.0f, WindowWidth - sprite.width);
            transform.y = std::clamp(transform.y, 0.0f, WindowHeight - sprite.height);

            sprite.tintR = static_cast<std::uint8_t>(std::min(255.0f, static_cast<float>(sprite.tintR) + 320.0f * deltaTime));
            sprite.tintG = static_cast<std::uint8_t>(std::min(255.0f, static_cast<float>(sprite.tintG) + 320.0f * deltaTime));
            sprite.tintB = static_cast<std::uint8_t>(std::min(255.0f, static_cast<float>(sprite.tintB) + 320.0f * deltaTime));
        });
}

void RunScreen::applyContactDamage(float deltaTime)
{
    auto* playerTransform = m_world.getComponent<TransformComponent>(m_playerEntity);
    auto* playerCollider = m_world.getComponent<ColliderComponent>(m_playerEntity);
    auto* playerHealth = m_world.getComponent<HealthComponent>(m_playerEntity);
    auto* playerSprite = m_world.getComponent<SpriteComponent>(m_playerEntity);

    if (!playerTransform || !playerCollider || !playerHealth || !playerSprite)
    {
        return;
    }

    const float playerCenterX = playerTransform->x + playerSprite->width * 0.5f;
    const float playerCenterY = playerTransform->y + playerSprite->height * 0.5f;

    m_world.forEach<EnemyComponent, TransformComponent, ColliderComponent, ContactDamageComponent>(
        [&](EntityId enemyEntity,
            EnemyComponent&,
            TransformComponent& enemyTransform,
            ColliderComponent& enemyCollider,
            ContactDamageComponent& contactDamage)
        {
            contactDamage.cooldownLeft = std::max(0.0f, contactDamage.cooldownLeft - deltaTime);

            auto* enemySprite = m_world.getComponent<SpriteComponent>(enemyEntity);
            const float enemyHalfW = enemySprite ? (enemySprite->width * 0.5f) : 0.0f;
            const float enemyHalfH = enemySprite ? (enemySprite->height * 0.5f) : 0.0f;
            const float enemyCenterX = enemyTransform.x + enemyHalfW;
            const float enemyCenterY = enemyTransform.y + enemyHalfH;

            const float dx = playerCenterX - enemyCenterX;
            const float dy = playerCenterY - enemyCenterY;
            const float minDistance = playerCollider->radius + enemyCollider.radius;

            if (dx * dx + dy * dy <= minDistance * minDistance && contactDamage.cooldownLeft <= 0.0f)
            {
                playerHealth->currentHp = std::max(0.0f, playerHealth->currentHp - contactDamage.damage);
                contactDamage.cooldownLeft = contactDamage.hitCooldown;
            }
        });
}

void RunScreen::updateAutoAttack(float deltaTime)
{
    auto* autoAttack = m_world.getComponent<AutoAttackComponent>(m_playerEntity);
    if (!autoAttack)
    {
        return;
    }

    autoAttack->cooldownLeft = std::max(0.0f, autoAttack->cooldownLeft - deltaTime);
    if (autoAttack->cooldownLeft > 0.0f)
    {
        return;
    }

    const float effectiveRange = autoAttack->range * m_runtimeStats.attackRangeMultiplier;
    const float effectiveDamage = autoAttack->damage * m_runtimeStats.damageMultiplier;
    const float effectiveInterval = autoAttack->interval * m_runtimeStats.attackIntervalMultiplier;

    const EntityId target = findNearestEnemyInRange(effectiveRange);
    if (target == InvalidEntity)
    {
        return;
    }

    applyDamageToEntity(target, effectiveDamage);
    autoAttack->cooldownLeft = effectiveInterval;
    m_hitFeedbackTimer = 0.18f;

    if (auto* targetSprite = m_world.getComponent<SpriteComponent>(target))
    {
        targetSprite->tintR = 255;
        targetSprite->tintG = 90;
        targetSprite->tintB = 90;
    }
}

void RunScreen::updatePickups(float deltaTime)
{
    auto* playerTransform = m_world.getComponent<TransformComponent>(m_playerEntity);
    auto* playerCollider = m_world.getComponent<ColliderComponent>(m_playerEntity);
    auto* playerSprite = m_world.getComponent<SpriteComponent>(m_playerEntity);
    auto* experience = m_world.getComponent<ExperienceComponent>(m_playerEntity);

    if (!playerTransform || !playerCollider || !playerSprite || !experience)
    {
        return;
    }

    m_world.forEach<LifetimeComponent>(
        [&](EntityId entity, LifetimeComponent& lifetime)
        {
            lifetime.remainingSeconds -= deltaTime;
            if (lifetime.remainingSeconds <= 0.0f)
            {
                m_world.destroyEntityDeferred(entity);
            }
        });

    const float playerCenterX = playerTransform->x + playerSprite->width * 0.5f;
    const float playerCenterY = playerTransform->y + playerSprite->height * 0.5f;

    int gainedXp = 0;
    m_world.forEach<PickupComponent, TransformComponent, ColliderComponent>(
        [&](EntityId pickupEntity, PickupComponent& pickup, TransformComponent& transform, ColliderComponent& collider)
        {
            if (pickup.type != PickupType::Xp)
            {
                return;
            }

            const float dx = playerCenterX - transform.x;
            const float dy = playerCenterY - transform.y;
            const float pickupRadius = playerCollider->radius + collider.radius;
            if (dx * dx + dy * dy <= pickupRadius * pickupRadius)
            {
                gainedXp += pickup.value;
                m_world.destroyEntityDeferred(pickupEntity);
            }
        });

    if (gainedXp > 0)
    {
        experience->currentXp += gainedXp;
    }
}

void RunScreen::handleDeaths()
{
    m_world.forEach<EnemyComponent, HealthComponent, TransformComponent>(
        [&](EntityId entity, EnemyComponent& enemy, HealthComponent& health, TransformComponent& transform)
        {
            if (health.currentHp > 0.0f)
            {
                return;
            }

            int xpReward = 0;
            if (enemy.archetype == EnemyArchetype::Melee)
            {
                xpReward = MeleeXpReward;
            }

            spawnXpPickup(transform.x + EnemyWidth * 0.5f, transform.y + EnemyHeight * 0.5f, xpReward);
            m_world.destroyEntityDeferred(entity);
        });
}

void RunScreen::handleLevelUpProgression()
{
    auto* experience = m_world.getComponent<ExperienceComponent>(m_playerEntity);
    if (!experience)
    {
        return;
    }

    while (experience->currentXp >= experience->xpToNext)
    {
        experience->currentXp -= experience->xpToNext;
        experience->level += 1;
        experience->xpToNext = computeXpToNext(experience->level);
        ++m_pendingLevelUps;
    }

    if (m_pendingLevelUps > 0 && !m_isLevelUpSelection)
    {
        m_isLevelUpSelection = true;
        rollLevelUpChoices();
    }
}

int RunScreen::computeXpToNext(int level) const
{
    return static_cast<int>(std::round(40.0 * std::pow(static_cast<double>(level), 1.25)));
}

void RunScreen::spawnXpPickup(float x, float y, int xpValue)
{
    if (xpValue <= 0)
    {
        return;
    }

    const EntityId pickup = m_world.createEntity();
    m_world.addComponent<TransformComponent>(pickup, TransformComponent{x - XpPickupWidth * 0.5f, y - XpPickupHeight * 0.5f});
    m_world.addComponent<SpriteComponent>(pickup, SpriteComponent{
        "xp_pickup",
        XpPickupWidth,
        XpPickupHeight,
        120,
        230,
        255,
        255
    });
    m_world.addComponent<ColliderComponent>(pickup, ColliderComponent{XpPickupColliderRadius});
    m_world.addComponent<PickupComponent>(pickup, PickupComponent{PickupType::Xp, xpValue});
    m_world.addComponent<LifetimeComponent>(pickup, LifetimeComponent{XpPickupLifetime});
}

EntityId RunScreen::findNearestEnemyInRange(float range)
{
    auto* playerTransform = m_world.getComponent<TransformComponent>(m_playerEntity);
    auto* playerSprite = m_world.getComponent<SpriteComponent>(m_playerEntity);
    if (!playerTransform || !playerSprite)
    {
        return InvalidEntity;
    }

    const float playerCenterX = playerTransform->x + playerSprite->width * 0.5f;
    const float playerCenterY = playerTransform->y + playerSprite->height * 0.5f;
    const float maxDistanceSquared = range * range;

    EntityId nearest = InvalidEntity;
    float nearestDistanceSquared = maxDistanceSquared;

    m_world.forEach<EnemyComponent, TransformComponent, HealthComponent>(
        [&](EntityId entity, EnemyComponent&, TransformComponent& transform, HealthComponent& health)
        {
            if (health.currentHp <= 0.0f)
            {
                return;
            }

            auto* sprite = m_world.getComponent<SpriteComponent>(entity);
            const float halfW = sprite ? (sprite->width * 0.5f) : 0.0f;
            const float halfH = sprite ? (sprite->height * 0.5f) : 0.0f;
            const float enemyCenterX = transform.x + halfW;
            const float enemyCenterY = transform.y + halfH;
            const float dx = playerCenterX - enemyCenterX;
            const float dy = playerCenterY - enemyCenterY;
            const float distanceSquared = dx * dx + dy * dy;

            if (distanceSquared <= nearestDistanceSquared)
            {
                nearestDistanceSquared = distanceSquared;
                nearest = entity;
            }
        });

    return nearest;
}

void RunScreen::applyDamageToEntity(EntityId entity, float damage)
{
    auto* health = m_world.getComponent<HealthComponent>(entity);
    if (!health)
    {
        return;
    }

    health->currentHp = std::max(0.0f, health->currentHp - damage);
}

std::size_t RunScreen::countAliveEnemies()
{
    std::size_t count = 0;
    m_world.forEach<EnemyComponent>(
        [&](EntityId, EnemyComponent&)
        {
            ++count;
        });
    return count;
}

void RunScreen::rollLevelUpChoices()
{
    const auto& perks = getAllPerks();
    std::vector<int> indices(perks.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), m_rng);

    for (std::size_t i = 0; i < m_levelChoices.size(); ++i)
    {
        const PerkDefinition& perk = perks[static_cast<std::size_t>(indices[i])];
        m_levelChoiceIds[i] = perk.id;
        m_levelChoices[i] = std::string(perk.name) + " - " + std::string(perk.description);
    }
}

void RunScreen::applyPerk(PerkId perkId)
{
    const PerkDefinition& perk = findPerkDefinition(perkId);
    m_lastSelectedPerk = std::string(perk.name);

    switch (perkId)
    {
        case PerkId::PowerI:
            m_runtimeStats.damageMultiplier *= 1.15f;
            break;
        case PerkId::AttackSpeedI:
            m_runtimeStats.attackIntervalMultiplier *= 0.88f;
            break;
        case PerkId::VitalityI:
        {
            auto* health = m_world.getComponent<HealthComponent>(m_playerEntity);
            if (health)
            {
                health->maxHp += 20.0f;
                health->currentHp = health->maxHp;
            }
            break;
        }
        case PerkId::CalmMind:
        {
            auto* arousal = m_world.getComponent<ArousalComponent>(m_playerEntity);
            if (arousal)
            {
                arousal->current = std::max(0.0f, arousal->current - 20.0f);
            }
            break;
        }
        case PerkId::Momentum:
            m_runtimeStats.speedMultiplier *= 1.10f;
            break;
        case PerkId::RangeI:
            m_runtimeStats.attackRangeMultiplier *= 1.18f;
            break;
    }
}

void RunScreen::updateHudText()
{
    if (!m_hasUiFont || !m_world.isAlive(m_playerEntity))
    {
        return;
    }

    const auto* health = m_world.getComponent<HealthComponent>(m_playerEntity);
    const auto* arousal = m_world.getComponent<ArousalComponent>(m_playerEntity);
    const auto* autoAttack = m_world.getComponent<AutoAttackComponent>(m_playerEntity);
    const auto* experience = m_world.getComponent<ExperienceComponent>(m_playerEntity);

    if (!health || !arousal || !autoAttack || !experience)
    {
        m_hudText.setString("HUD unavailable: missing player components");
        return;
    }

    const char* stateText = "Combat";
    if (m_isDefeated)
    {
        stateText = "Defeat";
        m_instruction.setString("Defeat (HP). Press Enter to return to base");
    }
    else if (m_isLevelUpSelection)
    {
        stateText = "Level Up";
        m_instruction.setString("Level Up pause. Choose 1/2/3");
    }
    else
    {
        m_instruction.setString("Move: WASD/Arrows  |  Esc: return to base");
    }

    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "State: " << stateText
       << "   HP: " << static_cast<int>(health->currentHp) << "/" << static_cast<int>(health->maxHp)
       << "   Arousal: " << static_cast<int>(arousal->current) << "/" << static_cast<int>(arousal->max)
       << "   Lvl: " << experience->level
       << "   XP: " << experience->currentXp << "/" << experience->xpToNext
       << "   Enemies: " << countAliveEnemies()
       << "   AutoAtk CD: " << autoAttack->cooldownLeft
       << "   Mult(Dmg/Int/Spd/Rng): "
       << m_runtimeStats.damageMultiplier << "/"
       << m_runtimeStats.attackIntervalMultiplier << "/"
       << m_runtimeStats.speedMultiplier << "/"
       << m_runtimeStats.attackRangeMultiplier;
    m_hudText.setString(ss.str());

    if (m_lastSelectedPerk.empty())
    {
        m_lastPerkText.setString("Last perk choice: none");
    }
    else
    {
        m_lastPerkText.setString("Last perk choice: " + m_lastSelectedPerk);
    }
}

void RunScreen::renderEntities(sf::RenderWindow& window)
{
    m_world.forEach<TransformComponent, SpriteComponent>(
        [&](EntityId, TransformComponent& transform, SpriteComponent& spriteData)
        {
            const sf::Texture& texture = m_assetManager.getTextureOrFallback(spriteData.textureId);

            sf::Sprite sprite;
            sprite.setTexture(texture);

            const sf::Vector2u textureSize = texture.getSize();
            if (textureSize.x > 0U && textureSize.y > 0U)
            {
                const float scaleX = spriteData.width / static_cast<float>(textureSize.x);
                const float scaleY = spriteData.height / static_cast<float>(textureSize.y);
                sprite.setScale(scaleX, scaleY);
            }

            sprite.setPosition(transform.x, transform.y);
            sprite.setColor(sf::Color(spriteData.tintR, spriteData.tintG, spriteData.tintB, spriteData.tintA));
            window.draw(sprite);
        });
}

void RunScreen::renderLevelUpOverlay(sf::RenderWindow& window)
{
    sf::RectangleShape overlay(sf::Vector2f(WindowWidth, WindowHeight));
    overlay.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(overlay);

    m_levelUpTitleText.setString("Level Up! Choose 1 of 3");
    window.draw(m_levelUpTitleText);

    std::ostringstream choices;
    choices << "1) " << m_levelChoices[0] << '\n'
            << "2) " << m_levelChoices[1] << '\n'
            << "3) " << m_levelChoices[2];
    m_levelUpChoicesText.setString(choices.str());
    window.draw(m_levelUpChoicesText);
}
