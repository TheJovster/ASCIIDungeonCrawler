#include "Game.h"
#include "Input.h"
#include <iostream>
#include <windows.h>

namespace DungeonGame {

    Game::Game() {

        std::ios::sync_with_stdio(false); 
        std::cin.tie(nullptr);

        m_dungeon.generate();
        spawnPlayer();
        // hide console cursor
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        SMALL_RECT windowSize = { 0, 0, MAP_WIDTH - 1, MAP_HEIGHT - 1 };
        COORD bufferSize = { MAP_WIDTH, MAP_HEIGHT };
        SetConsoleScreenBufferSize(consoleHandle, bufferSize);
        SetConsoleWindowInfo(consoleHandle, TRUE, &windowSize);
    }

    void Game::run() {
        while (m_running) {
            m_renderer.drawMap(m_dungeon, m_player);
            Action action = getInput();
            handleAction(action);
        }
    }

    void Game::spawnPlayer() {
        // spawn in center of first room — guaranteed to be floor
        const Room& first = m_dungeon.getRooms()[0];
        m_player.x = first.centerX();
        m_player.y = first.centerY();
    }

    void Game::handleAction(Action action) {
        int newX = m_player.x;
        int newY = m_player.y;

        switch (action) {
        case Action::MoveUp:    --newY; break;
        case Action::MoveDown:  ++newY; break;
        case Action::MoveLeft:  --newX; break;
        case Action::MoveRight: ++newX; break;
        case Action::Quit:      m_running = false; return;
        default: return;
        }

        if (isWalkable(newX, newY)) {
            m_player.x = newX;
            m_player.y = newY;
        }
    }

    bool Game::isWalkable(int x, int y) const {
        if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
            return false;

        const auto& tile = m_dungeon.getGrid()[y][x];
        return tile.type == TileType::Floor;
    }

}