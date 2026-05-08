#include "screens/BaseScreen.h"

#include "game/PersistentState.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <sstream>

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
    m_title.setString("Base Stub");
    m_title.setCharacterSize(48);
    m_title.setFillColor(sf::Color(230, 230, 245));
    m_title.setPosition(64.f, 64.f);

    m_instruction.setFont(uiFont);
    m_instruction.setString("Press Enter to start a run");
    m_instruction.setCharacterSize(28);
    m_instruction.setFillColor(sf::Color(180, 210, 255));
    m_instruction.setPosition(64.f, 140.f);

    m_totalText.setFont(uiFont);
    m_totalText.setCharacterSize(24);
    m_totalText.setFillColor(sf::Color(220, 240, 200));
    m_totalText.setPosition(64.f, 220.f);

    m_resultText.setFont(uiFont);
    m_resultText.setCharacterSize(22);
    m_resultText.setFillColor(sf::Color(220, 220, 245));
    m_resultText.setPosition(64.f, 260.f);

    m_hintText.setFont(uiFont);
    m_hintText.setCharacterSize(16);
    m_hintText.setFillColor(sf::Color(150, 160, 180));
    m_hintText.setString("Result is granted on return from run (Stop / Defeat). "
                         "Base only reads PersistentState.");
    m_hintText.setPosition(64.f, 310.f);

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
        window.draw(m_totalText);
        window.draw(m_resultText);
        window.draw(m_hintText);
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
        return;
    }

    std::ostringstream result;
    result << "Last run: raw " << m_persistentState.lastRunResourceRaw()
           << ", granted " << m_persistentState.lastRunResourceGranted()
           << ", outcome " << outcomeLabel(m_persistentState.lastRunOutcome());
    m_resultText.setString(result.str());
}
