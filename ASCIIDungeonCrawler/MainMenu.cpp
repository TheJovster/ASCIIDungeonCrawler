#include "MainMenu.h"
#include "Types.h"
#include <SFML/Graphics.hpp>
#include <string>

namespace DungeonGame {

    bool showMainMenu(sf::RenderWindow& window) {
        sf::Font font;
        font.loadFromFile("C:\\Windows\\Fonts\\consola.ttf");

        const int charSize = 20;
        const int centerX = SCREEN_WIDTH / 2;
        const int centerY = SCREEN_HEIGHT / 2;

        bool selected = true; // true = Start, false = Quit

        auto makeText = [&](const std::string& str, int x, int y,
            sf::Color color = sf::Color::White) {
                sf::Text t;
                t.setFont(font);
                t.setCharacterSize(charSize);
                t.setFillColor(color);
                t.setString(str);
                sf::FloatRect bounds = t.getLocalBounds();
                t.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
                t.setPosition((float)x, (float)y);
                return t;
            };

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                    return false;
                }
                if (event.type == sf::Event::KeyPressed) {
                    switch (event.key.code) {
                    case sf::Keyboard::Up:
                    case sf::Keyboard::Down:
                        selected = !selected;
                        break;
                    case sf::Keyboard::Return:
                        return selected;
                    case sf::Keyboard::Escape:
                        return false;
                    default: break;
                    }
                }
            }

            window.clear(sf::Color::Black);

            window.draw(makeText("ASCII DUNGEON CRAWLER", centerX, centerY - 80,
                sf::Color::Yellow));

            window.draw(makeText(
                selected ? "-> Start Game <-" : "   Start Game   ",
                centerX, centerY,
                selected ? sf::Color::Cyan : sf::Color::White));

            window.draw(makeText(
                !selected ? "-> Quit Game <-" : "   Quit Game   ",
                centerX, centerY + 40,
                !selected ? sf::Color::Cyan : sf::Color::White));

            window.draw(makeText("[Up/Dn] Select   [Enter] Confirm",
                centerX, centerY + 120, sf::Color(150, 150, 150)));

            window.display();
        }
        return false;
    }

}