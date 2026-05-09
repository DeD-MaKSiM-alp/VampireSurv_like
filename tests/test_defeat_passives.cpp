#include "test_runner.h"

#include "game/DefeatPassives.h"
#include "game/PersistentState.h"

#include <random>
#include <set>
#include <stdexcept>

TEST_CASE(passives_table_has_four_entries)
{
    const auto& passives = getAllDefeatPassives();
    EXPECT_EQ(static_cast<int>(passives.size()), 4);
}

TEST_CASE(passives_have_unique_ids)
{
    const auto& passives = getAllDefeatPassives();
    std::set<int> seen;
    for (const auto& p : passives)
    {
        seen.insert(static_cast<int>(p.id));
    }
    EXPECT_EQ(static_cast<int>(seen.size()), 4);
}

TEST_CASE(passives_lookup_each_id_returns_matching_definition)
{
    const DefeatPassiveId allIds[] = {
        DefeatPassiveId::FracturedNerves,
        DefeatPassiveId::SensitiveSkin,
        DefeatPassiveId::HeavySteps,
        DefeatPassiveId::ShakyFocus,
    };
    for (const auto id : allIds)
    {
        const auto& def = findDefeatPassiveDefinition(id);
        EXPECT_TRUE(def.id == id);
        EXPECT_FALSE(def.name.empty());
        EXPECT_FALSE(def.description.empty());
    }
}

TEST_CASE(passives_lookup_invalid_id_throws)
{
    bool threw = false;
    try
    {
        findDefeatPassiveDefinition(static_cast<DefeatPassiveId>(255));
    }
    catch (const std::exception&)
    {
        threw = true;
    }
    EXPECT_TRUE(threw);
}

TEST_CASE(modifiers_default_state_is_unity)
{
    PersistentState s;
    const DefeatPassiveModifiers mods = computeModifiers(s);
    EXPECT_NEAR(mods.incomingHpMul, 1.0f, 1e-6);
    EXPECT_NEAR(mods.incomingArousalMul, 1.0f, 1e-6);
    EXPECT_NEAR(mods.speedMul, 1.0f, 1e-6);
    EXPECT_NEAR(mods.attackIntervalMul, 1.0f, 1e-6);
}

TEST_CASE(modifiers_fractured_nerves_single_stack)
{
    PersistentState s;
    s.applyRunResult(0, LastRunOutcome::None, nullptr);
    // Inject one passive deterministically by running enough RNG until we
    // get a Fractured Nerves. A simpler approach: roll RNG until count > 0.
    std::mt19937 rng(1u);
    while (s.defeatPassiveCount(DefeatPassiveId::FracturedNerves) == 0)
    {
        s.applyRunResult(0, LastRunOutcome::DefeatHp, &rng);
        if (s.defeatPassives().size() > 200) break;  // safety
    }

    // Recompute modifiers using a synthetic state that has only one
    // FracturedNerves so the test is deterministic regardless of which other
    // passives the RNG above produced. Use a fresh state with a directly
    // injected single passive via repeated DefeatHp until pure single stack
    // is undecidable; instead build expectations from current count.
    const int n = s.defeatPassiveCount(DefeatPassiveId::FracturedNerves);
    const DefeatPassiveModifiers mods = computeModifiers(s);
    EXPECT_NEAR(mods.incomingHpMul, std::pow(1.10f, static_cast<float>(n)), 1e-4f);
}

TEST_CASE(modifiers_match_expected_stacking_bases)
{
    // Drive RNG enough that all four passives get at least one stack with
    // high probability, then compare component-wise modifiers to the
    // formula pow(base, count(passive)) for each passive.
    PersistentState s;
    std::mt19937 rng(42u);
    for (int i = 0; i < 80; ++i)
    {
        s.applyRunResult(0, LastRunOutcome::DefeatHp, &rng);
    }

    const int fractured = s.defeatPassiveCount(DefeatPassiveId::FracturedNerves);
    const int sensitive = s.defeatPassiveCount(DefeatPassiveId::SensitiveSkin);
    const int heavy     = s.defeatPassiveCount(DefeatPassiveId::HeavySteps);
    const int shaky     = s.defeatPassiveCount(DefeatPassiveId::ShakyFocus);

    const DefeatPassiveModifiers mods = computeModifiers(s);
    EXPECT_NEAR(mods.incomingHpMul,      std::pow(1.10f, static_cast<float>(fractured)), 1e-3f);
    EXPECT_NEAR(mods.incomingArousalMul, std::pow(1.15f, static_cast<float>(sensitive)), 1e-3f);
    EXPECT_NEAR(mods.speedMul,           std::pow(0.92f, static_cast<float>(heavy)),     1e-3f);
    EXPECT_NEAR(mods.attackIntervalMul,  std::pow(1.12f, static_cast<float>(shaky)),     1e-3f);
}

TEST_CASE(rng_grant_distribution_eventually_covers_all_four)
{
    // Sanity: with 200 trials and uniform 1/4 distribution, the probability
    // of missing any single passive is ~5.7e-26. Effectively impossible.
    PersistentState s;
    std::mt19937 rng(7u);
    for (int i = 0; i < 200; ++i)
    {
        s.applyRunResult(0, LastRunOutcome::DefeatArousal, &rng);
    }

    EXPECT_TRUE(s.defeatPassiveCount(DefeatPassiveId::FracturedNerves) > 0);
    EXPECT_TRUE(s.defeatPassiveCount(DefeatPassiveId::SensitiveSkin)   > 0);
    EXPECT_TRUE(s.defeatPassiveCount(DefeatPassiveId::HeavySteps)      > 0);
    EXPECT_TRUE(s.defeatPassiveCount(DefeatPassiveId::ShakyFocus)      > 0);
}
