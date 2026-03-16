#pragma once
#include <SFML/Graphics.hpp>
#include <string>

namespace DungeonGame {

    class Button {
    public:
        Button(const std::string& label, const sf::Texture& texture,
            const sf::Font& font, sf::Vector2f position, sf::Vector2f size,
            unsigned int charSize);

        void draw(sf::RenderWindow& window, bool selected, bool pressed = false) const;
        bool contains(sf::Vector2f point) const;

    private:
        sf::Sprite  m_sprite;
        sf::Text    m_text;
    };
}