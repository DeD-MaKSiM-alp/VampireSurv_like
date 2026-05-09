#include "test_runner.h"

#include "game/PersistentState.h"

#include <cstdio>
#include <fstream>
#include <random>
#include <string>

namespace
{
const std::string kSavePath = "test_save.txt";

void removeFile(const std::string& path)
{
    std::remove(path.c_str());
}

void writeRaw(const std::string& path, const std::string& contents)
{
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    out << contents;
}
}

TEST_CASE(save_load_default_state_roundtrip)
{
    removeFile(kSavePath);

    PersistentState a;
    EXPECT_TRUE(saveToFile(a, kSavePath));

    PersistentState b;
    EXPECT_TRUE(loadFromFile(b, kSavePath));

    EXPECT_EQ(b.totalResource(), 0);
    EXPECT_EQ(b.lastRunResourceRaw(), 0);
    EXPECT_EQ(b.lastRunResourceGranted(), 0);
    EXPECT_TRUE(b.lastRunOutcome() == LastRunOutcome::None);
    EXPECT_EQ(b.runsCount(), 0);
    EXPECT_EQ(b.hpDefeatsCount(), 0);
    EXPECT_EQ(b.arousalDefeatsCount(), 0);
    EXPECT_TRUE(b.defeatPassives().empty());

    removeFile(kSavePath);
}

TEST_CASE(save_load_populated_state_roundtrip)
{
    removeFile(kSavePath);

    PersistentState a;
    std::mt19937 rng(2024u);
    a.applyRunResult(20, LastRunOutcome::Stopped, &rng);
    a.applyRunResult(15, LastRunOutcome::DefeatHp, &rng);
    a.applyRunResult(8, LastRunOutcome::DefeatArousal, &rng);

    EXPECT_TRUE(saveToFile(a, kSavePath));

    PersistentState b;
    EXPECT_TRUE(loadFromFile(b, kSavePath));

    EXPECT_EQ(b.totalResource(), a.totalResource());
    EXPECT_EQ(b.lastRunResourceRaw(), a.lastRunResourceRaw());
    EXPECT_EQ(b.lastRunResourceGranted(), a.lastRunResourceGranted());
    EXPECT_TRUE(b.lastRunOutcome() == a.lastRunOutcome());
    EXPECT_EQ(b.runsCount(), 3);
    EXPECT_EQ(b.hpDefeatsCount(), 1);
    EXPECT_EQ(b.arousalDefeatsCount(), 1);
    EXPECT_EQ(static_cast<int>(b.defeatPassives().size()),
              static_cast<int>(a.defeatPassives().size()));

    // Same passive identities in same order.
    for (std::size_t i = 0; i < a.defeatPassives().size(); ++i)
    {
        EXPECT_TRUE(a.defeatPassives()[i] == b.defeatPassives()[i]);
    }

    removeFile(kSavePath);
}

TEST_CASE(load_missing_file_returns_false_and_keeps_state)
{
    removeFile(kSavePath);

    PersistentState s;
    s.applyRunResult(50, LastRunOutcome::Stopped, nullptr);

    const int totalBefore = s.totalResource();
    EXPECT_FALSE(loadFromFile(s, kSavePath));
    // State is untouched on failure.
    EXPECT_EQ(s.totalResource(), totalBefore);
    EXPECT_EQ(s.runsCount(), 1);
}

TEST_CASE(load_corrupt_missing_version_returns_false_and_keeps_state)
{
    removeFile(kSavePath);
    writeRaw(kSavePath,
        "total 100\n"
        "runs 5\n");

    PersistentState s;
    s.applyRunResult(7, LastRunOutcome::Stopped, nullptr);
    const int totalBefore = s.totalResource();

    EXPECT_FALSE(loadFromFile(s, kSavePath));
    EXPECT_EQ(s.totalResource(), totalBefore);
    EXPECT_EQ(s.runsCount(), 1);

    removeFile(kSavePath);
}

TEST_CASE(load_corrupt_wrong_version_returns_false)
{
    removeFile(kSavePath);
    writeRaw(kSavePath,
        "version 999\n"
        "total 50\n");

    PersistentState s;
    EXPECT_FALSE(loadFromFile(s, kSavePath));
    EXPECT_EQ(s.totalResource(), 0);

    removeFile(kSavePath);
}

TEST_CASE(load_corrupt_garbage_value_returns_false)
{
    removeFile(kSavePath);
    writeRaw(kSavePath,
        "version 1\n"
        "total notanumber\n");

    PersistentState s;
    EXPECT_FALSE(loadFromFile(s, kSavePath));
    EXPECT_EQ(s.totalResource(), 0);

    removeFile(kSavePath);
}

TEST_CASE(load_corrupt_unknown_outcome_returns_false)
{
    removeFile(kSavePath);
    writeRaw(kSavePath,
        "version 1\n"
        "lastOutcome SomethingElse\n");

    PersistentState s;
    EXPECT_FALSE(loadFromFile(s, kSavePath));

    removeFile(kSavePath);
}

TEST_CASE(load_corrupt_unknown_passive_returns_false)
{
    removeFile(kSavePath);
    writeRaw(kSavePath,
        "version 1\n"
        "passives FracturedNerves Bogus\n");

    PersistentState s;
    EXPECT_FALSE(loadFromFile(s, kSavePath));

    removeFile(kSavePath);
}

TEST_CASE(load_unknown_keys_are_ignored)
{
    removeFile(kSavePath);
    writeRaw(kSavePath,
        "version 1\n"
        "total 42\n"
        "futureField 99\n"
        "anotherUnknown some text\n"
        "runs 3\n");

    PersistentState s;
    EXPECT_TRUE(loadFromFile(s, kSavePath));
    EXPECT_EQ(s.totalResource(), 42);
    EXPECT_EQ(s.runsCount(), 3);

    removeFile(kSavePath);
}

TEST_CASE(load_empty_passives_line_is_valid)
{
    removeFile(kSavePath);
    writeRaw(kSavePath,
        "version 1\n"
        "total 5\n"
        "passives\n");

    PersistentState s;
    EXPECT_TRUE(loadFromFile(s, kSavePath));
    EXPECT_EQ(s.totalResource(), 5);
    EXPECT_TRUE(s.defeatPassives().empty());

    removeFile(kSavePath);
}

TEST_CASE(load_empty_file_returns_false)
{
    removeFile(kSavePath);
    writeRaw(kSavePath, "");

    PersistentState s;
    EXPECT_FALSE(loadFromFile(s, kSavePath));

    removeFile(kSavePath);
}

TEST_CASE(save_creates_human_readable_text)
{
    removeFile(kSavePath);

    PersistentState a;
    a.applyRunResult(13, LastRunOutcome::Stopped, nullptr);
    EXPECT_TRUE(saveToFile(a, kSavePath));

    std::ifstream in(kSavePath);
    std::string firstLine;
    std::getline(in, firstLine);
    EXPECT_TRUE(firstLine == "version 1");

    removeFile(kSavePath);
}
