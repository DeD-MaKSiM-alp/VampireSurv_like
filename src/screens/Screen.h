#pragma once

#include "app/GameState.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <optional>

class Screen
{
public:
    virtual ~Screen() = default;

    virtual std::optional<GameState> handleEvent(const sf::Event& event) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
};
