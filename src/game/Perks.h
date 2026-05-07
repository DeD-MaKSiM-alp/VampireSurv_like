#pragma once

#include <string_view>
#include <vector>

enum class PerkId
{
    PowerI,
    AttackSpeedI,
    VitalityI,
    CalmMind,
    Momentum,
    RangeI
};

struct PerkDefinition
{
    PerkId id{};
    std::string_view name;
    std::string_view description;
};

const std::vector<PerkDefinition>& getAllPerks();
const PerkDefinition& findPerkDefinition(PerkId id);
