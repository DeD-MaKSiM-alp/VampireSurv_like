#include "assets/AssetManager.h"

#include <SFML/Graphics/Image.hpp>

AssetManager::AssetManager()
    : m_fallbackTexture(createFallbackTexture())
{
}

bool AssetManager::loadTexture(const std::string& key, const std::string& path)
{
    sf::Texture texture;
    if (texture.loadFromFile(path))
    {
        m_textures[key] = std::move(texture);
        return true;
    }

    m_textures[key] = m_fallbackTexture;
    return false;
}

bool AssetManager::hasTexture(const std::string& key) const
{
    return m_textures.find(key) != m_textures.end();
}

const sf::Texture* AssetManager::getTexture(const std::string& key) const
{
    const auto it = m_textures.find(key);
    if (it == m_textures.end())
    {
        return nullptr;
    }

    return &it->second;
}

const sf::Texture& AssetManager::getTextureOrFallback(const std::string& key) const
{
    if (const sf::Texture* texture = getTexture(key))
    {
        return *texture;
    }

    return m_fallbackTexture;
}

sf::Texture AssetManager::createFallbackTexture() const
{
    sf::Image image;
    constexpr unsigned int size = 32;
    image.create(size, size, sf::Color(255, 0, 255));

    for (unsigned int y = 0; y < size; ++y)
    {
        for (unsigned int x = 0; x < size; ++x)
        {
            if (((x / 8U) + (y / 8U)) % 2U == 0U)
            {
                image.setPixel(x, y, sf::Color(50, 50, 50));
            }
        }
    }

    sf::Texture texture;
    texture.loadFromImage(image);
    return texture;
}
