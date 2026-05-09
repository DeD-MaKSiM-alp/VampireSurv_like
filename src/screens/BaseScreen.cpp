#include "screens/BaseScreen.h"

#include "game/DefeatPassives.h"
#include "game/PersistentState.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <sstream>
#include <string>

namespace
{
const char* outcomeLabel(LastRunOutcome outcome)
{
    switch (outcome)
    {
        case LastRunOutcome::Stopped:       return "Stopped";
        case LastRunOutcome::DefeatHp:      return "DefeatHp";
        case LastRunOutcome::DefeatArousal: return "DefeatArousal";
        case LastRunOutcome::None:          return "None";
    }
    return "Unknown";
}

std::string formatPassiveList(const PersistentState& state)
{
    const auto& all = getAllDefeatPassives();
    std::ostringstream out;
    out << "Defeat passives:";

    bool any = false;
    for (const DefeatPassiveDefinition& def : all)
    {
        const int count = state.defeatPassiveCount(def.id);
        if (count <= 0)
        {
            continue;
        }
        out << "\n  - " << def.name;
        if (count > 1)
        {
            out << " x" << count;
        }
        any = true;
    }

    if (!any)
    {
        return "Defeat passives: none";
    }
    return out.str();
}
}

BaseScreen::BaseScreen(const sf::Font& uiFont, bool hasUiFont, const PersistentState& persistentState)
    : m_hasUiFont(hasUiFont)
    , m_persistentState(persistentState)
{
    if (!m_hasUiFont)
    {
        return;
    }

    m_title.setFont(uiFont);
    m_title.setString("Base");
    m_title.setCharacterSize(48);
    m_title.setFillColor(sf::Color(230, 230, 245));
    m_title.setPosition(64.f, 48.f);

    m_instruction.setFont(uiFont);
    m_instruction.setString("Press Enter to start a run");
    m_instruction.setCharacterSize(26);
    m_instruction.setFillColor(sf::Color(180, 210, 255));
    m_instruction.setPosition(64.f, 116.f);

    // Stats block
    m_statsText.setFont(uiFont);
    m_statsText.setCharacterSize(22);
    m_statsText.setFillColor(sf::Color(210, 220, 240));
    m_statsText.setPosition(64.f, 188.f);

    // Resources block
    m_totalText.setFont(uiFont);
    m_totalText.setCharacterSize(22);
    m_totalText.setFillColor(sf::Color(220, 240, 200));
    m_totalText.setPosition(64.f, 252.f);

    m_resultText.setFont(uiFont);
    m_resultText.setCharacterSize(20);
    m_resultText.setFillColor(sf::Color(220, 220, 245));
    m_resultText.setPosition(64.f, 286.f);

    // Defeat passives block
    m_passivesText.setFont(uiFont);
    m_passivesText.setCharacterSize(20);
    m_passivesText.setFillColor(sf::Color(255, 200, 200));
    m_passivesText.setPosition(64.f, 350.f);

    refreshResultText();
}

std::optional<GameState> BaseScreen::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
    {
        return GameState::Run;
    }

    return std::nullopt;
}

void BaseScreen::update(float)
{
    refreshResultText();
}

void BaseScreen::render(sf::RenderWindow& window)
{
    window.clear(sf::Color(25, 24, 34));

    if (m_hasUiFont)
    {
        window.draw(m_title);
        window.draw(m_instruction);
        window.draw(m_statsText);
        window.draw(m_totalText);
        window.draw(m_resultText);
        window.draw(m_passivesText);
    }
}

void BaseScreen::refreshResultText()
{
    if (!m_hasUiFont)
    {
        return;
    }

    std::ostringstream total;
    total << "Total resource: " << m_persistentState.totalResource();
    m_totalText.setString(total.str());

    if (m_persistentState.lastRunOutcome() == LastRunOutcome::None)
    {
        m_resultText.setString("Last run: No runs yet");
    }
    else
    {
        std::ostringstream result;
        result << "Last run: raw " << m_persistentState.lastRunResourceRaw()
               << ", granted " << m_persistentState.lastRunResourceGranted()
               << ", outcome " << outcomeLabel(m_persistentState.lastRunOutcome());
        m_resultText.setString(result.str());
    }

    m_passivesText.setString(formatPassiveList(m_persistentState));

    std::ostringstream stats;
    stats << "Runs: " << m_persistentState.runsCount()
          << "   HP defeats: " << m_persistentState.hpDefeatsCount()
          << "   Arousal defeats: " << m_persistentState.arousalDefeatsCount();
    m_statsText.setString(stats.str());
}
