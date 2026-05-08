#pragma once

#include "game/DefeatPassives.h"

#include <cstdint>
#include <random>
#include <string>
#include <vector>

enum class LastRunOutcome : std::uint8_t
{
    None,
    Stopped,
    DefeatHp,
    DefeatArousal
};

class PersistentState
{
public:
    void applyRunResult(int rawResource, LastRunOutcome outcome, std::mt19937* rng);

    int totalResource() const { return m_totalResource; }
    int lastRunResourceRaw() const { return m_lastRunResourceRaw; }
    int lastRunResourceGranted() const { return m_lastRunResourceGranted; }
    LastRunOutcome lastRunOutcome() const { return m_lastRunOutcome; }

    int runsCount() const { return m_runsCount; }
    int hpDefeatsCount() const { return m_hpDefeatsCount; }
    int arousalDefeatsCount() const { return m_arousalDefeatsCount; }

    const std::vector<DefeatPassiveId>& defeatPassives() const { return m_defeatPassives; }
    int defeatPassiveCount(DefeatPassiveId id) const;

    // Friend access for SaveSystem load path: keeps the public surface
    // narrow while letting the file parser populate the state in one place.
    friend bool loadFromFile(PersistentState& state, const std::string& path);

private:
    void grantRandomDefeatPassive(std::mt19937& rng);

    int m_totalResource{0};
    int m_lastRunResourceRaw{0};
    int m_lastRunResourceGranted{0};
    LastRunOutcome m_lastRunOutcome{LastRunOutcome::None};
    int m_runsCount{0};
    int m_hpDefeatsCount{0};
    int m_arousalDefeatsCount{0};
    std::vector<DefeatPassiveId> m_defeatPassives;
};

bool saveToFile(const PersistentState& state, const std::string& path);
bool loadFromFile(PersistentState& state, const std::string& path);
