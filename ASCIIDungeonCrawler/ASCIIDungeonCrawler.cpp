#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#include "Game.h"
#include "MainMenu.h"
#include "AudioManager.h"
#include "Types.h"
#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode(DungeonGame::SCREEN_WIDTH, DungeonGame::SCREEN_HEIGHT),
        "ASCII Dungeon Crawler");
    window.setFramerateLimit(30);

    while (window.isOpen()) {
        DungeonGame::AudioManager::get().playMusic(DungeonGame::MusicTrack::Menu);
        if (!DungeonGame::showMainMenu(window)) {
            window.close();
            return 0;
        }
        DungeonGame::Game game;
        game.run(window);
    }
    return 0;
}