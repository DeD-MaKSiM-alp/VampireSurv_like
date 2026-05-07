#pragma once

#include <SFML/Graphics/Texture.hpp>

#include <string>
#include <unordered_map>

class AssetManager
{
public:
    AssetManager();

    bool loadTexture(const std::string& key, const std::string& path);
    bool hasTexture(const std::string& key) const;
    const sf::Texture* getTexture(const std::string& key) const;
    const sf::Texture& getTextureOrFallback(const std::string& key) const;

private:
    sf::Texture createFallbackTexture() const;

    sf::Texture m_fallbackTexture;
    std::unordered_map<std::string, sf::Texture> m_textures;
};
