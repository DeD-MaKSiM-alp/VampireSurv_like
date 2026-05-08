#pragma once

#include <cstdint>

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
    void applyRunResult(int rawResource, LastRunOutcome outcome);

    int totalResource() const { return m_totalResource; }
    int lastRunResourceRaw() const { return m_lastRunResourceRaw; }
    int lastRunResourceGranted() const { return m_lastRunResourceGranted; }
    LastRunOutcome lastRunOutcome() const { return m_lastRunOutcome; }

private:
    int m_totalResource{0};
    int m_lastRunResourceRaw{0};
    int m_lastRunResourceGranted{0};
    LastRunOutcome m_lastRunOutcome{LastRunOutcome::None};
};
