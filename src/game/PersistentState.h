#pragma once

#include "game/DefeatPassives.h"

#include <cstdint>
#include <random>
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

    const std::vector<DefeatPassiveId>& defeatPassives() const { return m_defeatPassives; }
    int defeatPassiveCount(DefeatPassiveId id) const;

private:
    void grantRandomDefeatPassive(std::mt19937& rng);

    int m_totalResource{0};
    int m_lastRunResourceRaw{0};
    int m_lastRunResourceGranted{0};
    LastRunOutcome m_lastRunOutcome{LastRunOutcome::None};
    std::vector<DefeatPassiveId> m_defeatPassives;
};
