#include "game/PersistentState.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr float DefeatGrantFraction = 0.3f;
}

void PersistentState::applyRunResult(int rawResource, LastRunOutcome outcome, std::mt19937* rng)
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

    // Defeat-only passive grant: Stopped/None never produce passives. Esc-during-
    // active-run is treated as Stopped per stage 12 policy, so it stays out of
    // this branch as well. Duplicates are intentionally allowed (MVP choice) and
    // stack multiplicatively via pow(base, count) inside computeModifiers().
    if (rng && (outcome == LastRunOutcome::DefeatHp || outcome == LastRunOutcome::DefeatArousal))
    {
        grantRandomDefeatPassive(*rng);
    }
}

int PersistentState::defeatPassiveCount(DefeatPassiveId id) const
{
    int count = 0;
    for (const DefeatPassiveId& entry : m_defeatPassives)
    {
        if (entry == id)
        {
            ++count;
        }
    }
    return count;
}

void PersistentState::grantRandomDefeatPassive(std::mt19937& rng)
{
    const auto& all = getAllDefeatPassives();
    if (all.empty())
    {
        return;
    }

    std::uniform_int_distribution<std::size_t> dist(0u, all.size() - 1u);
    const std::size_t index = dist(rng);
    m_defeatPassives.push_back(all[index].id);
}
