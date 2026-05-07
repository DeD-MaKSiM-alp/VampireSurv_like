#include "screens/BaseScreen.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

BaseScreen::BaseScreen(const sf::Font& uiFont, bool hasUiFont)
    : m_hasUiFont(hasUiFont)
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
}

void BaseScreen::render(sf::RenderWindow& window)
{
    window.clear(sf::Color(25, 24, 34));

    if (m_hasUiFont)
    {
        window.draw(m_title);
        window.draw(m_instruction);
    }
}
