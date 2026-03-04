#include "Game.h"
#include "MainMenu.h"
#include <SFML/Graphics.hpp>
#include "Types.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(DungeonGame::SCREEN_WIDTH, DungeonGame::SCREEN_HEIGHT), "ASCII Dungeon Crawler");
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        // main menu goes here later
        DungeonGame::Game game;
        game.run(window);
    }
    return 0;
}