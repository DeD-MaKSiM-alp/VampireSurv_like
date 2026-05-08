#include "game/PersistentState.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace
{
constexpr float DefeatGrantFraction = 0.3f;
constexpr int SaveFormatVersion = 1;

const char* outcomeToString(LastRunOutcome outcome)
{
    switch (outcome)
    {
        case LastRunOutcome::None:          return "None";
        case LastRunOutcome::Stopped:       return "Stopped";
        case LastRunOutcome::DefeatHp:      return "DefeatHp";
        case LastRunOutcome::DefeatArousal: return "DefeatArousal";
    }
    return "None";
}

bool parseOutcome(const std::string& token, LastRunOutcome& out)
{
    if (token == "None")          { out = LastRunOutcome::None;          return true; }
    if (token == "Stopped")       { out = LastRunOutcome::Stopped;       return true; }
    if (token == "DefeatHp")      { out = LastRunOutcome::DefeatHp;      return true; }
    if (token == "DefeatArousal") { out = LastRunOutcome::DefeatArousal; return true; }
    return false;
}

const char* passiveToString(DefeatPassiveId id)
{
    switch (id)
    {
        case DefeatPassiveId::FracturedNerves: return "FracturedNerves";
        case DefeatPassiveId::SensitiveSkin:   return "SensitiveSkin";
        case DefeatPassiveId::HeavySteps:      return "HeavySteps";
        case DefeatPassiveId::ShakyFocus:      return "ShakyFocus";
    }
    return "Unknown";
}

bool parsePassive(const std::string& token, DefeatPassiveId& out)
{
    if (token == "FracturedNerves") { out = DefeatPassiveId::FracturedNerves; return true; }
    if (token == "SensitiveSkin")   { out = DefeatPassiveId::SensitiveSkin;   return true; }
    if (token == "HeavySteps")      { out = DefeatPassiveId::HeavySteps;      return true; }
    if (token == "ShakyFocus")      { out = DefeatPassiveId::ShakyFocus;      return true; }
    return false;
}
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

    // Run / defeat counters are bumped here so all run-result book-keeping
    // sits in a single place. None never bumps anything (no run actually
    // happened). Stopped only bumps runsCount.
    if (outcome != LastRunOutcome::None)
    {
        ++m_runsCount;
    }
    if (outcome == LastRunOutcome::DefeatHp)
    {
        ++m_hpDefeatsCount;
    }
    else if (outcome == LastRunOutcome::DefeatArousal)
    {
        ++m_arousalDefeatsCount;
    }

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

bool saveToFile(const PersistentState& state, const std::string& path)
{
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out)
    {
        return false;
    }

    out << "version " << SaveFormatVersion << '\n';
    out << "total " << state.totalResource() << '\n';
    out << "lastRaw " << state.lastRunResourceRaw() << '\n';
    out << "lastGranted " << state.lastRunResourceGranted() << '\n';
    out << "lastOutcome " << outcomeToString(state.lastRunOutcome()) << '\n';
    out << "runs " << state.runsCount() << '\n';
    out << "hpDefeats " << state.hpDefeatsCount() << '\n';
    out << "arousalDefeats " << state.arousalDefeatsCount() << '\n';

    out << "passives";
    for (const DefeatPassiveId id : state.defeatPassives())
    {
        out << ' ' << passiveToString(id);
    }
    out << '\n';

    return out.good();
}

bool loadFromFile(PersistentState& state, const std::string& path)
{
    std::ifstream in(path);
    if (!in)
    {
        std::cerr << "[save] no file, fresh state\n";
        return false;
    }

    PersistentState parsed;
    bool versionSeen = false;

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::istringstream ss(line);
        std::string key;
        if (!(ss >> key))
        {
            continue;
        }

        if (key == "version")
        {
            int v = -1;
            if (!(ss >> v) || v != SaveFormatVersion)
            {
                std::cerr << "[save] corrupted, using defaults\n";
                return false;
            }
            versionSeen = true;
        }
        else if (key == "total")
        {
            if (!(ss >> parsed.m_totalResource))
            {
                std::cerr << "[save] corrupted, using defaults\n";
                return false;
            }
        }
        else if (key == "lastRaw")
        {
            if (!(ss >> parsed.m_lastRunResourceRaw))
            {
                std::cerr << "[save] corrupted, using defaults\n";
                return false;
            }
        }
        else if (key == "lastGranted")
        {
            if (!(ss >> parsed.m_lastRunResourceGranted))
            {
                std::cerr << "[save] corrupted, using defaults\n";
                return false;
            }
        }
        else if (key == "lastOutcome")
        {
            std::string token;
            if (!(ss >> token) || !parseOutcome(token, parsed.m_lastRunOutcome))
            {
                std::cerr << "[save] corrupted, using defaults\n";
                return false;
            }
        }
        else if (key == "runs")
        {
            if (!(ss >> parsed.m_runsCount))
            {
                std::cerr << "[save] corrupted, using defaults\n";
                return false;
            }
        }
        else if (key == "hpDefeats")
        {
            if (!(ss >> parsed.m_hpDefeatsCount))
            {
                std::cerr << "[save] corrupted, using defaults\n";
                return false;
            }
        }
        else if (key == "arousalDefeats")
        {
            if (!(ss >> parsed.m_arousalDefeatsCount))
            {
                std::cerr << "[save] corrupted, using defaults\n";
                return false;
            }
        }
        else if (key == "passives")
        {
            std::string token;
            while (ss >> token)
            {
                DefeatPassiveId id{};
                if (!parsePassive(token, id))
                {
                    std::cerr << "[save] corrupted, using defaults\n";
                    return false;
                }
                parsed.m_defeatPassives.push_back(id);
            }
        }
        // Unknown keys are intentionally ignored to allow forward-compatible files.
    }

    if (!versionSeen)
    {
        std::cerr << "[save] corrupted, using defaults\n";
        return false;
    }

    state = parsed;
    return true;
}
