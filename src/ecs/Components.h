#pragma once

#include <cstdint>
#include <string>

struct TransformComponent
{
    float x{0.0f};
    float y{0.0f};
};

struct VelocityComponent
{
    float vx{0.0f};
    float vy{0.0f};
};

struct SpriteComponent
{
    std::string textureId;
    float width{0.0f};
    float height{0.0f};
    std::uint8_t tintR{255};
    std::uint8_t tintG{255};
    std::uint8_t tintB{255};
    std::uint8_t tintA{255};
};

struct ColliderComponent
{
    float radius{0.0f};
};

struct HealthComponent
{
    float currentHp{100.0f};
    float maxHp{100.0f};
};

struct ArousalComponent
{
    float current{0.0f};
    float max{100.0f};
};

struct PlayerTagComponent
{
};

enum class EnemyArchetype : std::uint8_t
{
    Melee
};

struct EnemyComponent
{
    EnemyArchetype archetype{EnemyArchetype::Melee};
    float moveSpeed{140.0f};
};

struct ContactDamageComponent
{
    float damage{8.0f};
    float hitCooldown{0.9f};
    float cooldownLeft{0.0f};
};

struct AutoAttackComponent
{
    float damage{16.0f};
    float range{260.0f};
    float interval{0.55f};
    float cooldownLeft{0.0f};
};

struct ExperienceComponent
{
    int level{1};
    int currentXp{0};
    int xpToNext{40};
};

enum class PickupType : std::uint8_t
{
    Xp
};

struct PickupComponent
{
    PickupType type{PickupType::Xp};
    int value{0};
};

struct LifetimeComponent
{
    float remainingSeconds{0.0f};
};
