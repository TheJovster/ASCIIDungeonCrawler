#include "Game.h"
#include "MainMenu.h"

int main() {
    while (true) {
        if (!DungeonGame::showMainMenu())
            return 0;

        DungeonGame::Game game;
        game.run();
        // when run() returns, we're back at the menu
    }
}