#include "game/Perks.h"

#include <stdexcept>

const std::vector<PerkDefinition>& getAllPerks()
{
    static const std::vector<PerkDefinition> perks = {
        {PerkId::PowerI, "Power I", "+15% damage"},
        {PerkId::AttackSpeedI, "Attack Speed I", "-12% attack interval"},
        {PerkId::VitalityI, "Vitality I", "+20 max HP and full heal"},
        {PerkId::CalmMind, "Calm Mind", "-20 current arousal"},
        {PerkId::Momentum, "Momentum", "+10% movement speed"},
        {PerkId::RangeI, "Range I", "+18% attack range"},
    };
    return perks;
}

const PerkDefinition& findPerkDefinition(PerkId id)
{
    for (const PerkDefinition& perk : getAllPerks())
    {
        if (perk.id == id)
        {
            return perk;
        }
    }

    throw std::runtime_error("Unknown perk id.");
}
