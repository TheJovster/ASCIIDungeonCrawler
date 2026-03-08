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

    while (window.isOpen()) {
        DungeonGame::AudioManager::get().playMusic(DungeonGame::MusicTrack::Menu);
        if (!DungeonGame::showMainMenu(window)) {
            window.close();
            return 0;
        }
        DungeonGame::Game game(window);
        game.run(window);
    }
    return 0;
}