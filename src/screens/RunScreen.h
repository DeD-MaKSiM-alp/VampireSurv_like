#pragma once

#include "screens/Screen.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

class RunScreen final : public Screen
{
public:
    RunScreen(const sf::Font& uiFont, bool hasUiFont);

    std::optional<GameState> handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    bool m_hasUiFont{false};
    sf::Text m_title;
    sf::Text m_instruction;
};
