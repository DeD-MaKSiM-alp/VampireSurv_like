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
