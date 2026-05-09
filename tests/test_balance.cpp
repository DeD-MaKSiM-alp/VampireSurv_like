#include "test_runner.h"

#include "game/Balance.h"

#include <cmath>

TEST_CASE(xp_threshold_level_one_is_forty)
{
    // round(40 * 1^1.25) = 40
    EXPECT_EQ(balance::xpToNextLevel(1), 40);
}

TEST_CASE(xp_threshold_known_levels)
{
    // round(40 * 2^1.25) = round(95.137) = 95
    EXPECT_EQ(balance::xpToNextLevel(2), 95);
    // round(40 * 3^1.25) = round(157.912) = 158
    EXPECT_EQ(balance::xpToNextLevel(3), 158);
    // round(40 * 5^1.25) = round(299.066) = 299
    EXPECT_EQ(balance::xpToNextLevel(5), 299);
    // round(40 * 10^1.25) = round(711.477) = 711
    EXPECT_EQ(balance::xpToNextLevel(10), 711);
}

TEST_CASE(xp_threshold_is_strictly_monotonic)
{
    int previous = balance::xpToNextLevel(1);
    for (int level = 2; level <= 30; ++level)
    {
        const int v = balance::xpToNextLevel(level);
        EXPECT_TRUE(v > previous);
        previous = v;
    }
}

TEST_CASE(xp_threshold_zero_or_negative_clamped_to_level_one)
{
    EXPECT_EQ(balance::xpToNextLevel(0), 40);
    EXPECT_EQ(balance::xpToNextLevel(-5), 40);
}
