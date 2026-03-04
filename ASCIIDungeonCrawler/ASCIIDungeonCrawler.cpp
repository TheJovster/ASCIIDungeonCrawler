#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#include "Game.h"
#include "MainMenu.h"
#include "Types.h"
#include <SFML/Graphics.hpp>
#include "Windows.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(DungeonGame::SCREEN_WIDTH, DungeonGame::SCREEN_HEIGHT),
        "ASCII Dungeon Crawler");
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        if (!DungeonGame::showMainMenu(window)) {
            window.close();
            return 0;
        }
        DungeonGame::Game game;
        game.run(window);
    }
    return 0;
}