#include "game/PersistentState.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr float DefeatGrantFraction = 0.3f;
}

void PersistentState::applyRunResult(int rawResource, LastRunOutcome outcome)
{
    const int safeRaw = std::max(0, rawResource);
    int granted = 0;

    switch (outcome)
    {
        case LastRunOutcome::Stopped:
            granted = safeRaw;
            break;
        case LastRunOutcome::DefeatHp:
        case LastRunOutcome::DefeatArousal:
            granted = static_cast<int>(std::floor(static_cast<float>(safeRaw) * DefeatGrantFraction));
            break;
        case LastRunOutcome::None:
            granted = 0;
            break;
    }

    m_lastRunResourceRaw = safeRaw;
    m_lastRunResourceGranted = granted;
    m_lastRunOutcome = outcome;
    m_totalResource += granted;
}
