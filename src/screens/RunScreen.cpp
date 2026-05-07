#include "screens/RunScreen.h"

#include "ecs/Components.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <cmath>
#include <sstream>

namespace
{
constexpr float WindowWidth = 1280.0f;
constexpr float WindowHeight = 720.0f;
constexpr float PlayerSpeed = 220.0f;
constexpr float PlayerWidth = 48.0f;
constexpr float PlayerHeight = 48.0f;
}

RunScreen::RunScreen(const sf::Font& uiFont, bool hasUiFont)
    : m_hasUiFont(hasUiFont)
{
    m_assetManager.loadTexture("player", "assets/textures/placeholders/player.png");
    createPlayerEntity();

    if (m_hasUiFont)
    {
        m_title.setFont(uiFont);
        m_title.setString("Run Stub");
        m_title.setCharacterSize(46);
        m_title.setFillColor(sf::Color(245, 230, 220));
        m_title.setPosition(32.f, 16.f);

        m_instruction.setFont(uiFont);
        m_instruction.setString("Move: WASD/Arrows  |  Esc: return to base");
        m_instruction.setCharacterSize(22);
        m_instruction.setFillColor(sf::Color(255, 205, 170));
        m_instruction.setPosition(32.f, 64.f);

        m_hudText.setFont(uiFont);
        m_hudText.setCharacterSize(20);
        m_hudText.setFillColor(sf::Color(200, 235, 255));
        m_hudText.setPosition(32.f, 100.f);
    }

    updateHudText();
}

std::optional<GameState> RunScreen::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
    {
        return GameState::Base;
    }

    return std::nullopt;
}

void RunScreen::update(float deltaTime)
{
    updatePlayerMovement(deltaTime);
    updateHudText();
}

void RunScreen::render(sf::RenderWindow& window)
{
    window.clear(sf::Color(32, 22, 20));
    renderEntities(window);

    if (m_hasUiFont)
    {
        window.draw(m_title);
        window.draw(m_instruction);
        window.draw(m_hudText);
    }
}

void RunScreen::createPlayerEntity()
{
    m_playerEntity = m_world.createEntity();

    m_world.addComponent<PlayerTagComponent>(m_playerEntity);
    m_world.addComponent<TransformComponent>(m_playerEntity, TransformComponent{
        (WindowWidth - PlayerWidth) * 0.5f,
        (WindowHeight - PlayerHeight) * 0.5f
    });
    m_world.addComponent<VelocityComponent>(m_playerEntity, VelocityComponent{0.0f, 0.0f});
    m_world.addComponent<SpriteComponent>(m_playerEntity, SpriteComponent{
        "player",
        PlayerWidth,
        PlayerHeight,
        255,
        255,
        255,
        255
    });
    m_world.addComponent<HealthComponent>(m_playerEntity, HealthComponent{100.0f, 100.0f});
    m_world.addComponent<ArousalComponent>(m_playerEntity, ArousalComponent{0.0f, 100.0f});
}

void RunScreen::updatePlayerMovement(float deltaTime)
{
    if (!m_world.isAlive(m_playerEntity))
    {
        return;
    }

    auto* transform = m_world.getComponent<TransformComponent>(m_playerEntity);
    auto* velocity = m_world.getComponent<VelocityComponent>(m_playerEntity);
    auto* sprite = m_world.getComponent<SpriteComponent>(m_playerEntity);

    if (!transform || !velocity || !sprite)
    {
        return;
    }

    float directionX = 0.0f;
    float directionY = 0.0f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
        directionX -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
        directionX += 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    {
        directionY -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
        directionY += 1.0f;
    }

    const float lengthSquared = directionX * directionX + directionY * directionY;
    if (lengthSquared > 0.0f)
    {
        const float invLength = 1.0f / std::sqrt(lengthSquared);
        directionX *= invLength;
        directionY *= invLength;
    }

    velocity->vx = directionX * PlayerSpeed;
    velocity->vy = directionY * PlayerSpeed;

    transform->x += velocity->vx * deltaTime;
    transform->y += velocity->vy * deltaTime;

    transform->x = std::clamp(transform->x, 0.0f, WindowWidth - sprite->width);
    transform->y = std::clamp(transform->y, 0.0f, WindowHeight - sprite->height);
}

void RunScreen::updateHudText()
{
    if (!m_hasUiFont || !m_world.isAlive(m_playerEntity))
    {
        return;
    }

    const auto* health = m_world.getComponent<HealthComponent>(m_playerEntity);
    const auto* arousal = m_world.getComponent<ArousalComponent>(m_playerEntity);
    const auto* velocity = m_world.getComponent<VelocityComponent>(m_playerEntity);

    if (!health || !arousal || !velocity)
    {
        m_hudText.setString("HUD unavailable: missing player components");
        return;
    }

    std::ostringstream ss;
    ss << "HP: " << static_cast<int>(health->currentHp) << "/" << static_cast<int>(health->maxHp)
       << "   Arousal: " << static_cast<int>(arousal->current) << "/" << static_cast<int>(arousal->max)
       << "   Vel: (" << static_cast<int>(velocity->vx) << ", " << static_cast<int>(velocity->vy) << ")";
    m_hudText.setString(ss.str());
}

void RunScreen::renderEntities(sf::RenderWindow& window)
{
    m_world.forEach<TransformComponent, SpriteComponent>(
        [&](EntityId, TransformComponent& transform, SpriteComponent& spriteData)
        {
            const sf::Texture& texture = m_assetManager.getTextureOrFallback(spriteData.textureId);

            sf::Sprite sprite;
            sprite.setTexture(texture);

            const sf::Vector2u textureSize = texture.getSize();
            if (textureSize.x > 0U && textureSize.y > 0U)
            {
                const float scaleX = spriteData.width / static_cast<float>(textureSize.x);
                const float scaleY = spriteData.height / static_cast<float>(textureSize.y);
                sprite.setScale(scaleX, scaleY);
            }

            sprite.setPosition(transform.x, transform.y);
            sprite.setColor(sf::Color(spriteData.tintR, spriteData.tintG, spriteData.tintB, spriteData.tintA));
            window.draw(sprite);
        });
}
