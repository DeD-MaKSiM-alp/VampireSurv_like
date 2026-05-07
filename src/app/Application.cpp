#include "app/Application.h"

#include "screens/BaseScreen.h"
#include "screens/RunScreen.h"
#include "screens/Screen.h"

#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <array>

namespace
{
constexpr unsigned int WindowWidth = 1280;
constexpr unsigned int WindowHeight = 720;
constexpr unsigned int FrameRateLimit = 120;

const std::array<const char*, 3> UiFontCandidates = {
    "assets/fonts/arial.ttf",
    "C:/Windows/Fonts/segoeui.ttf",
    "C:/Windows/Fonts/arial.ttf",
};
}

Application::Application()
    : m_window(sf::VideoMode(WindowWidth, WindowHeight), "VampireSurvLike - Base")
{
    m_window.setFramerateLimit(FrameRateLimit);
    m_hasUiFont = loadUiFont();
    changeState(GameState::Base);
}

Application::~Application() = default;

void Application::run()
{
    sf::Clock clock;

    while (m_window.isOpen())
    {
        const float deltaTime = clock.restart().asSeconds();

        processEvents();
        update(deltaTime);
        render();
    }
}

void Application::processEvents()
{
    sf::Event event{};
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            m_window.close();
            continue;
        }

        if (!m_screen)
        {
            continue;
        }

        if (const auto requestedState = m_screen->handleEvent(event))
        {
            changeState(*requestedState);
        }
    }
}

void Application::update(float deltaTime)
{
    if (m_screen)
    {
        m_screen->update(deltaTime);
    }
}

void Application::render()
{
    m_window.clear();

    if (m_screen)
    {
        m_screen->render(m_window);
    }

    m_window.display();
}

void Application::changeState(GameState nextState)
{
    m_state = nextState;

    switch (m_state)
    {
        case GameState::Base:
            m_window.setTitle("VampireSurvLike - Base");
            m_screen = std::make_unique<BaseScreen>(m_uiFont, m_hasUiFont);
            break;
        case GameState::Run:
            m_window.setTitle("VampireSurvLike - Run");
            m_screen = std::make_unique<RunScreen>(m_uiFont, m_hasUiFont);
            break;
    }
}

bool Application::loadUiFont()
{
    for (const char* path : UiFontCandidates)
    {
        if (m_uiFont.loadFromFile(path))
        {
            return true;
        }
    }

    return false;
}
