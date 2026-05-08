#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

class PersistentState;

enum class DefeatPassiveId : std::uint8_t
{
    FracturedNerves,
    SensitiveSkin,
    HeavySteps,
    ShakyFocus
};

struct DefeatPassiveDefinition
{
    DefeatPassiveId id{};
    std::string_view name;
    std::string_view description;
};

struct DefeatPassiveModifiers
{
    float incomingHpMul{1.0f};
    float incomingArousalMul{1.0f};
    float speedMul{1.0f};
    float attackIntervalMul{1.0f};
};

const std::vector<DefeatPassiveDefinition>& getAllDefeatPassives();
const DefeatPassiveDefinition& findDefeatPassiveDefinition(DefeatPassiveId id);

DefeatPassiveModifiers computeModifiers(const PersistentState& state);
