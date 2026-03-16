#include "MainMenu.h"
#include "Button.h"
#include "HowToPlay.h"
#include "Types.h"
#include <SFML/Graphics.hpp>
#include <string>

namespace DungeonGame {

    bool showMainMenu(sf::RenderWindow& window) {
        sf::Font font;
        font.loadFromFile("C:\\Windows\\Fonts\\consola.ttf");

        sf::Texture buttonTexture;
        buttonTexture.loadFromFile("assets/button.png");

        const float screenW = (float)window.getSize().x;
        const float screenH = (float)window.getSize().y;
        const float centerX = screenW * 0.5f;
        const float centerY = screenH * 0.5f;

        const float texW = (float)buttonTexture.getSize().x;
        const float texH = (float)buttonTexture.getSize().y;
        const float aspect = texH / texW;
        const float btnW = screenW * MENU_BUTTON_WIDTH_PCT; //for 12% of the screen width
        const float btnH = btnW * aspect;

        const float btnX = centerX - btnW * 0.5f;
        const unsigned int charSize = 20;

        // layout constants
        const float gap = btnH + MENU_BUTTON_GAP;                      // space between buttons
        const float totalH = btnH * 3 + MENU_BUTTON_GAP * 2;           // total height of button block
        const float blockTop = centerY - totalH * 0.5f;                 // center the block vertically

        Button btnStart("Start Game", buttonTexture, font, { btnX, blockTop }, { btnW, btnH }, charSize);
        Button btnHowTo("How To Play", buttonTexture, font, { btnX, blockTop + gap }, { btnW, btnH }, charSize);
        Button btnQuit("Quit Game", buttonTexture, font, { btnX, blockTop + gap * 2.f }, { btnW, btnH }, charSize);

        Button* buttons[] = { &btnStart, &btnHowTo, &btnQuit };
        const int buttonCount = 3;

        int  selected = 0;
        bool pressed = false;
        
        auto makeText = [&](const std::string& str, float x, float y,
            sf::Color color = sf::Color::White) {
                sf::Text t;
                t.setFont(font);
                t.setCharacterSize(charSize);
                t.setFillColor(color);
                t.setString(str);
                sf::FloatRect bounds = t.getLocalBounds();
                t.setOrigin(bounds.width * 0.5f, bounds.height * 0.5f);
                t.setPosition(x, y);
                return t;
            };

        while (window.isOpen()) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            bool isHoveringSelected = buttons[selected]->contains(mousePos);
            pressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);

            // mouse hover
            for (int i = 0; i < buttonCount; ++i) {
                if (buttons[i]->contains(mousePos))
                    selected = i;
            }

            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                    return false;
                }

                // keyboard navigation
                if (event.type == sf::Event::KeyPressed) {
                    switch (event.key.code) {
                    case sf::Keyboard::Up:
                    case sf::Keyboard::W:
                        selected = std::max(0, selected - 1);
                        break;
                    case sf::Keyboard::Down:
                    case sf::Keyboard::S:
                        selected = std::min(buttonCount - 1, selected + 1);
                        break;
                    case sf::Keyboard::Return:
                    case sf::Keyboard::Space:
                        if (selected == 0) return true;
                        if (selected == 1) { showHowToPlay(window); break; }
                        if (selected == 2) return false;
                        break;
                    }
                }

                // mouse click
                if (event.type == sf::Event::MouseButtonReleased &&
                    event.mouseButton.button == sf::Mouse::Left) {
                    for (int i = 0; i < buttonCount; ++i) {
                        if (buttons[i]->contains(mousePos)) {
                            if (i == 0) return true;
                            if (i == 1) { showHowToPlay(window); break; }
                            if (i == 2) return false;
                        }
                    }
                }
            }

            window.clear(sf::Color::Black);

            window.draw(makeText("ASCII DUNGEON CRAWLER", centerX, blockTop - 60.f,
                sf::Color::Yellow));

            for (int i = 0; i < buttonCount; ++i)
                buttons[i]->draw(window, selected == i, pressed && isHoveringSelected && selected == i);

            window.draw(makeText(
                "[Up/Dn] or [W/S] Select   [Enter] or [Space] Confirm",
                centerX, blockTop + totalH + 30.f, sf::Color(150, 150, 150)));

            window.display();
        }
        return false;
    }
}