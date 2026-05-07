#include "screens/RunScreen.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

RunScreen::RunScreen(const sf::Font& uiFont, bool hasUiFont)
    : m_hasUiFont(hasUiFont)
{
    if (!m_hasUiFont)
    {
        return;
    }

    m_title.setFont(uiFont);
    m_title.setString("Run Stub");
    m_title.setCharacterSize(48);
    m_title.setFillColor(sf::Color(245, 230, 220));
    m_title.setPosition(64.f, 64.f);

    m_instruction.setFont(uiFont);
    m_instruction.setString("Press Escape to return to base");
    m_instruction.setCharacterSize(28);
    m_instruction.setFillColor(sf::Color(255, 205, 170));
    m_instruction.setPosition(64.f, 140.f);
}

std::optional<GameState> RunScreen::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
    {
        return GameState::Base;
    }

    return std::nullopt;
}

void RunScreen::update(float)
{
}

void RunScreen::render(sf::RenderWindow& window)
{
    window.clear(sf::Color(32, 22, 20));

    if (m_hasUiFont)
    {
        window.draw(m_title);
        window.draw(m_instruction);
    }
}
