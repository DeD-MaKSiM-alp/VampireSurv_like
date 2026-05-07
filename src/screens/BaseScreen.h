#pragma once

#include "screens/Screen.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

class BaseScreen final : public Screen
{
public:
    BaseScreen(const sf::Font& uiFont, bool hasUiFont);

    std::optional<GameState> handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    bool m_hasUiFont{false};
    sf::Text m_title;
    sf::Text m_instruction;
};
