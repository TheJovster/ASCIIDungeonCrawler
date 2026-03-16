#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#include "Game.h"
#include "MainMenu.h"
#include "AudioManager.h"
#include "Types.h"
#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(),
        "ASCII Dungeon Crawler", sf::Style::None);
    window.setFramerateLimit(60);

    sf::Font font;
    font.loadFromFile("C:\\Windows\\Fonts\\consola.ttf");

    while (window.isOpen()) {
        DungeonGame::AudioManager::get().playMusic(DungeonGame::MusicTrack::Menu);
        if (!DungeonGame::showMainMenu(window)) {
            window.close();
            return 0;
        }

        // loading screen
        window.clear(sf::Color::Black);
        sf::Text loading;
        loading.setFont(font);
        loading.setString("Loading...");
        loading.setCharacterSize(24);
        loading.setFillColor(sf::Color::White);
        sf::FloatRect bounds = loading.getLocalBounds();
        loading.setOrigin(bounds.width * 0.5f, bounds.height * 0.5f);
        loading.setPosition(
            window.getSize().x * 0.5f,
            window.getSize().y * 0.5f);
        window.draw(loading);
        window.display();

        DungeonGame::Game game(window);
        game.run(window);
    }
    return 0;
}