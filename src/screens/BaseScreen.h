#pragma once

#include "screens/Screen.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

class PersistentState;

class BaseScreen final : public Screen
{
public:
    BaseScreen(const sf::Font& uiFont, bool hasUiFont, const PersistentState& persistentState);

    std::optional<GameState> handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    void refreshResultText();

    bool m_hasUiFont{false};
    const PersistentState& m_persistentState;
    sf::Text m_title;
    sf::Text m_instruction;
    sf::Text m_resultText;
    sf::Text m_totalText;
    sf::Text m_statsText;
    sf::Text m_passivesText;
    sf::Text m_hintText;
};
