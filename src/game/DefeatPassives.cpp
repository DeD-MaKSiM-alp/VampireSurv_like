#include "game/DefeatPassives.h"

#include "game/PersistentState.h"

#include <cmath>
#include <stdexcept>

namespace
{
// MVP-fixed multiplicative stacking bases. Duplicates of the same passive are
// allowed and compound as pow(base, count). If the GDD changes the rule to
// additive or capped stacks, update the bases AND the comment in one place.
constexpr float FracturedNervesBase = 1.10f;
constexpr float SensitiveSkinBase   = 1.15f;
constexpr float HeavyStepsBase      = 0.92f;
constexpr float ShakyFocusBase      = 1.12f;
}

const std::vector<DefeatPassiveDefinition>& getAllDefeatPassives()
{
    static const std::vector<DefeatPassiveDefinition> passives = {
        {DefeatPassiveId::FracturedNerves, "Fractured Nerves", "+10% incoming HP damage"},
        {DefeatPassiveId::SensitiveSkin,   "Sensitive Skin",   "+15% incoming arousal damage"},
        {DefeatPassiveId::HeavySteps,      "Heavy Steps",      "-8% movement speed"},
        {DefeatPassiveId::ShakyFocus,      "Shaky Focus",      "+12% attack interval"},
    };
    return passives;
}

const DefeatPassiveDefinition& findDefeatPassiveDefinition(DefeatPassiveId id)
{
    for (const DefeatPassiveDefinition& passive : getAllDefeatPassives())
    {
        if (passive.id == id)
        {
            return passive;
        }
    }

    throw std::runtime_error("Unknown defeat passive id.");
}

DefeatPassiveModifiers computeModifiers(const PersistentState& state)
{
    const int fractured = state.defeatPassiveCount(DefeatPassiveId::FracturedNerves);
    const int sensitive = state.defeatPassiveCount(DefeatPassiveId::SensitiveSkin);
    const int heavy     = state.defeatPassiveCount(DefeatPassiveId::HeavySteps);
    const int shaky     = state.defeatPassiveCount(DefeatPassiveId::ShakyFocus);

    DefeatPassiveModifiers mods;
    mods.incomingHpMul      = std::pow(FracturedNervesBase, static_cast<float>(fractured));
    mods.incomingArousalMul = std::pow(SensitiveSkinBase,   static_cast<float>(sensitive));
    mods.speedMul           = std::pow(HeavyStepsBase,      static_cast<float>(heavy));
    mods.attackIntervalMul  = std::pow(ShakyFocusBase,      static_cast<float>(shaky));
    return mods;
}
