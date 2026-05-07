#pragma once

#include "app/GameState.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <memory>

class Screen;

class Application
{
public:
    Application();
    ~Application();

    void run();

private:
    void processEvents();
    void update(float deltaTime);
    void render();
    void changeState(GameState nextState);
    bool loadUiFont();

    sf::RenderWindow m_window;
    sf::Font m_uiFont;
    bool m_hasUiFont{false};
    GameState m_state{GameState::Base};
    std::unique_ptr<Screen> m_screen;
};
