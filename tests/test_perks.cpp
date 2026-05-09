#include "test_runner.h"

#include "game/Perks.h"

#include <set>
#include <stdexcept>

TEST_CASE(perks_table_has_six_entries)
{
    const auto& perks = getAllPerks();
    EXPECT_EQ(static_cast<int>(perks.size()), 6);
}

TEST_CASE(perks_have_unique_ids)
{
    const auto& perks = getAllPerks();
    std::set<int> seen;
    for (const auto& p : perks)
    {
        seen.insert(static_cast<int>(p.id));
    }
    EXPECT_EQ(static_cast<int>(seen.size()), 6);
}

TEST_CASE(perks_lookup_each_id_returns_matching_definition)
{
    const PerkId allIds[] = {
        PerkId::PowerI,
        PerkId::AttackSpeedI,
        PerkId::VitalityI,
        PerkId::CalmMind,
        PerkId::Momentum,
        PerkId::RangeI,
    };
    for (const PerkId id : allIds)
    {
        const auto& def = findPerkDefinition(id);
        EXPECT_TRUE(def.id == id);
        EXPECT_FALSE(def.name.empty());
        EXPECT_FALSE(def.description.empty());
    }
}

TEST_CASE(perks_lookup_invalid_id_throws)
{
    bool threw = false;
    try
    {
        // Pick a value beyond the enum range. Casting an out-of-range int
        // to enum is implementation-defined but works for our purposes
        // since the lookup uses linear scan and never matches.
        findPerkDefinition(static_cast<PerkId>(999));
    }
    catch (const std::exception&)
    {
        threw = true;
    }
    EXPECT_TRUE(threw);
}
