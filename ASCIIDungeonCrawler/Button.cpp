#include "Button.h"

namespace DungeonGame {

    Button::Button(const std::string& label, const sf::Texture& texture,
        const sf::Font& font, sf::Vector2f position, sf::Vector2f size,
        unsigned int charSize) {

        // sprite
        m_sprite.setTexture(texture);
        sf::Vector2u texSize = texture.getSize();
        m_sprite.setScale(size.x / texSize.x, size.y / texSize.y);
        m_sprite.setPosition(position);

        // text — centered on button
        m_text.setFont(font);
        m_text.setCharacterSize(charSize);
        m_text.setString(label);
        m_text.setFillColor(sf::Color::White);
        sf::FloatRect bounds = m_text.getLocalBounds();
        m_text.setOrigin(bounds.width * 0.5f, bounds.height * 0.5f);
        m_text.setPosition(
            position.x + size.x * 0.5f,
            position.y + size.y * 0.5f - bounds.height * 0.5f
        );
    }

    void Button::draw(sf::RenderWindow& window, bool selected, bool pressed) const {
        sf::Sprite s = m_sprite;
        if (pressed)
            s.setColor(sf::Color(150, 150, 150));
        else if (selected)
            s.setColor(sf::Color(255, 255, 255));
        else
            s.setColor(sf::Color(200, 200, 200));
        window.draw(s);
        window.draw(m_text);
    }

    bool Button::contains(sf::Vector2f point) const {
        return m_sprite.getGlobalBounds().contains(point);
    }
}