#pragma once

#include "assets/AssetManager.h"
#include "ecs/World.h"
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
    void createPlayerEntity();
    void updatePlayerMovement(float deltaTime);
    void updateHudText();
    void renderEntities(sf::RenderWindow& window);

    bool m_hasUiFont{false};
    AssetManager m_assetManager;
    World m_world;
    EntityId m_playerEntity{InvalidEntity};
    sf::Text m_title;
    sf::Text m_instruction;
    sf::Text m_hudText;
};
