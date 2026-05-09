#include "test_runner.h"

#include "game/PersistentState.h"

#include <random>

namespace
{
std::mt19937 makeRng()
{
    return std::mt19937(12345u);
}
}

TEST_CASE(state_default_is_clean)
{
    PersistentState s;
    EXPECT_EQ(s.totalResource(), 0);
    EXPECT_EQ(s.lastRunResourceRaw(), 0);
    EXPECT_EQ(s.lastRunResourceGranted(), 0);
    EXPECT_TRUE(s.lastRunOutcome() == LastRunOutcome::None);
    EXPECT_EQ(s.runsCount(), 0);
    EXPECT_EQ(s.hpDefeatsCount(), 0);
    EXPECT_EQ(s.arousalDefeatsCount(), 0);
    EXPECT_TRUE(s.defeatPassives().empty());
}

TEST_CASE(state_stopped_grants_full_resource)
{
    PersistentState s;
    auto rng = makeRng();
    s.applyRunResult(17, LastRunOutcome::Stopped, &rng);

    EXPECT_EQ(s.lastRunResourceRaw(), 17);
    EXPECT_EQ(s.lastRunResourceGranted(), 17);
    EXPECT_EQ(s.totalResource(), 17);
    EXPECT_EQ(s.runsCount(), 1);
    EXPECT_EQ(s.hpDefeatsCount(), 0);
    EXPECT_EQ(s.arousalDefeatsCount(), 0);
    EXPECT_TRUE(s.defeatPassives().empty());
    EXPECT_TRUE(s.lastRunOutcome() == LastRunOutcome::Stopped);
}

TEST_CASE(state_defeat_hp_grants_floor_30_and_passive)
{
    PersistentState s;
    auto rng = makeRng();
    s.applyRunResult(11, LastRunOutcome::DefeatHp, &rng);

    // floor(11 * 0.3) == 3
    EXPECT_EQ(s.lastRunResourceGranted(), 3);
    EXPECT_EQ(s.totalResource(), 3);
    EXPECT_EQ(s.runsCount(), 1);
    EXPECT_EQ(s.hpDefeatsCount(), 1);
    EXPECT_EQ(s.arousalDefeatsCount(), 0);
    EXPECT_EQ(static_cast<int>(s.defeatPassives().size()), 1);
}

TEST_CASE(state_defeat_arousal_grants_floor_30_and_passive)
{
    PersistentState s;
    auto rng = makeRng();
    s.applyRunResult(7, LastRunOutcome::DefeatArousal, &rng);

    // floor(7 * 0.3) == 2
    EXPECT_EQ(s.lastRunResourceGranted(), 2);
    EXPECT_EQ(s.totalResource(), 2);
    EXPECT_EQ(s.arousalDefeatsCount(), 1);
    EXPECT_EQ(s.hpDefeatsCount(), 0);
    EXPECT_EQ(static_cast<int>(s.defeatPassives().size()), 1);
}

TEST_CASE(state_defeat_without_rng_does_not_grant_passive)
{
    PersistentState s;
    s.applyRunResult(5, LastRunOutcome::DefeatHp, nullptr);

    EXPECT_EQ(s.lastRunResourceGranted(), 1);
    EXPECT_EQ(s.hpDefeatsCount(), 1);
    EXPECT_TRUE(s.defeatPassives().empty());
}

TEST_CASE(state_stopped_with_rng_does_not_grant_passive)
{
    PersistentState s;
    auto rng = makeRng();
    s.applyRunResult(10, LastRunOutcome::Stopped, &rng);
    EXPECT_TRUE(s.defeatPassives().empty());
}

TEST_CASE(state_none_outcome_changes_nothing_substantive)
{
    PersistentState s;
    auto rng = makeRng();
    s.applyRunResult(20, LastRunOutcome::None, &rng);

    EXPECT_EQ(s.lastRunResourceGranted(), 0);
    EXPECT_EQ(s.totalResource(), 0);
    EXPECT_EQ(s.runsCount(), 0);
    EXPECT_TRUE(s.defeatPassives().empty());
}

TEST_CASE(state_negative_raw_is_clamped_to_zero)
{
    PersistentState s;
    auto rng = makeRng();
    s.applyRunResult(-5, LastRunOutcome::Stopped, &rng);

    EXPECT_EQ(s.lastRunResourceRaw(), 0);
    EXPECT_EQ(s.lastRunResourceGranted(), 0);
    EXPECT_EQ(s.totalResource(), 0);
    EXPECT_EQ(s.runsCount(), 1);
}

TEST_CASE(state_zero_raw_is_handled_for_all_outcomes)
{
    PersistentState s;
    auto rng = makeRng();
    s.applyRunResult(0, LastRunOutcome::Stopped, &rng);
    s.applyRunResult(0, LastRunOutcome::DefeatHp, &rng);
    s.applyRunResult(0, LastRunOutcome::DefeatArousal, &rng);

    EXPECT_EQ(s.totalResource(), 0);
    EXPECT_EQ(s.runsCount(), 3);
    EXPECT_EQ(s.hpDefeatsCount(), 1);
    EXPECT_EQ(s.arousalDefeatsCount(), 1);
    EXPECT_EQ(static_cast<int>(s.defeatPassives().size()), 2);
}

TEST_CASE(state_defeat_floor_boundary_below_one_third)
{
    PersistentState s;
    s.applyRunResult(1, LastRunOutcome::DefeatHp, nullptr);  // floor(0.3) == 0
    EXPECT_EQ(s.lastRunResourceGranted(), 0);
    EXPECT_EQ(s.totalResource(), 0);

    PersistentState s2;
    s2.applyRunResult(3, LastRunOutcome::DefeatHp, nullptr);  // floor(0.9) == 0
    EXPECT_EQ(s2.lastRunResourceGranted(), 0);

    PersistentState s3;
    s3.applyRunResult(4, LastRunOutcome::DefeatHp, nullptr);  // floor(1.2) == 1
    EXPECT_EQ(s3.lastRunResourceGranted(), 1);

    PersistentState s4;
    s4.applyRunResult(10, LastRunOutcome::DefeatHp, nullptr);  // floor(3.0) == 3
    EXPECT_EQ(s4.lastRunResourceGranted(), 3);
}

TEST_CASE(state_multi_run_accumulates_total)
{
    PersistentState s;
    s.applyRunResult(5, LastRunOutcome::Stopped, nullptr);    // +5
    s.applyRunResult(10, LastRunOutcome::DefeatHp, nullptr);  // +3
    s.applyRunResult(20, LastRunOutcome::Stopped, nullptr);   // +20
    s.applyRunResult(7, LastRunOutcome::DefeatArousal, nullptr); // +2

    EXPECT_EQ(s.totalResource(), 30);
    EXPECT_EQ(s.runsCount(), 4);
    EXPECT_EQ(s.hpDefeatsCount(), 1);
    EXPECT_EQ(s.arousalDefeatsCount(), 1);
}

TEST_CASE(state_passive_count_matches_grants)
{
    PersistentState s;
    auto rng = makeRng();
    for (int i = 0; i < 10; ++i)
    {
        s.applyRunResult(0, LastRunOutcome::DefeatHp, &rng);
    }

    int total = 0;
    total += s.defeatPassiveCount(DefeatPassiveId::FracturedNerves);
    total += s.defeatPassiveCount(DefeatPassiveId::SensitiveSkin);
    total += s.defeatPassiveCount(DefeatPassiveId::HeavySteps);
    total += s.defeatPassiveCount(DefeatPassiveId::ShakyFocus);
    EXPECT_EQ(total, 10);
    EXPECT_EQ(static_cast<int>(s.defeatPassives().size()), 10);
}
