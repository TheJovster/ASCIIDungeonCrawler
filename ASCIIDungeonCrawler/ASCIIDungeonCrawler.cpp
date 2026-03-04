#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#include "Game.h"
#include "MainMenu.h"
#include "Types.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

int main() {
    
    sf::RenderWindow window(sf::VideoMode(DungeonGame::SCREEN_WIDTH, DungeonGame::SCREEN_HEIGHT),
        "ASCII Dungeon Crawler");
    window.setFramerateLimit(30);
    //music block - probably a bad place but works for now (testing)
    sf::Music menuMusic;
    menuMusic.openFromFile("assets/MenuMusic.ogg");
    menuMusic.setLoop(true);
    menuMusic.play();

    while (window.isOpen()) {
        if (!DungeonGame::showMainMenu(window)) {
            window.close();
            return 0;
        }
        menuMusic.stop();

        DungeonGame::Game game;
        game.run(window);
        menuMusic.play();
    }
    return 0;
}