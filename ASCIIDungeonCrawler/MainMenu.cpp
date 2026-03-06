#include "MainMenu.h"
#include "Types.h"
#include <SFML/Graphics.hpp>
#include <string>

namespace DungeonGame {

    

    bool showMainMenu(sf::RenderWindow& window) {
        sf::Font font;
        font.loadFromFile("C:\\Windows\\Fonts\\consola.ttf");

        enum class Selection
        {
            MainMenu,
            HowToPlay,
            QuitGame
        };


        const int charSize = 20;
        const int centerX = SCREEN_WIDTH / 2;
        const int centerY = SCREEN_HEIGHT / 2;
        int selected = 0;


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
                        selected = std::max(0, selected - 1);
                        break;
                    case sf::Keyboard::Down:
                        selected = std::min(2, selected + 1);
                        break;
                    case sf::Keyboard::Return:
                        if (selected == 0) return true;  // start Game
                        if (selected == 1) {
                            showHowToPlay(window);
                            break; // return to main menu after
                        }
                        if (selected == 2) return false; // quit
                        break;
                    }
                }
            }

            window.clear(sf::Color::Black);

            window.draw(makeText("ASCII DUNGEON CRAWLER", centerX, centerY - 80,
                sf::Color::Yellow));

            window.draw(makeText(
                selected == 0 ? "-> Start Game <-" : "   Start Game   ",
                centerX, centerY,
                selected == 0 ? sf::Color::Cyan : sf::Color::White));

            window.draw(makeText(
                selected == 1 ? "->How To Play<-" : "   How To Play   ",
                centerX, centerY + 40,
                selected == 1 ? sf::Color::Cyan : sf::Color::White));

            window.draw(makeText(
                selected == 2 ? "-> Quit Game <-" : "   Quit Game   ",
                centerX, centerY + 80,
                selected == 2 ? sf::Color::Cyan : sf::Color::White));

            window.draw(makeText("[Up/Dn] Select   [Enter] Confirm",
                centerX, centerY + 120, sf::Color(150, 150, 150)));

            window.display();
        }
        return false;
    }

}