#include "Game.h"
#include <windows.h>
#include <iostream>

namespace DungeonGame {

    Game::Game() {
        std::ios::sync_with_stdio(false);
        std::cin.tie(nullptr);

        m_dungeon.generate();
        spawnPlayer();

        //lock console map buffer size 
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

        SMALL_RECT minWindow = { 0, 0, 1, 1 };
        SetConsoleWindowInfo(consoleHandle, TRUE, &minWindow);

        COORD bufferSize = { (SHORT)CONSOLE_WIDTH, (SHORT)MAP_HEIGHT };
        SetConsoleScreenBufferSize(consoleHandle, bufferSize);

        SMALL_RECT windowSize = { 0, 0, (SHORT)(CONSOLE_WIDTH - 1), (SHORT)(MAP_HEIGHT - 1) };
        SetConsoleWindowInfo(consoleHandle, TRUE, &windowSize);

        // hide cursor
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(consoleHandle, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(consoleHandle, &cursorInfo);
    }

    void Game::run() {
        while (m_running) {
            m_renderer.drawMap(m_dungeon, m_player, m_log, m_state);
            m_renderer.drawHUD(m_player, m_state, m_activeEnemy, m_floor);
            Action action = getInput();

            switch (m_state) {
            case GameState::Exploring: handleExploring(action); break;
            case GameState::Combat:    handleCombat(action);    break;
            case GameState::GameOver:  m_running = false;       break;
            }
        }
    }

    void Game::spawnPlayer() {
        const Room& first = m_dungeon.getRooms()[0];
        m_player.x = first.centerX();
        m_player.y = first.centerY();
    }

    void Game::handleExploring(Action action) {
        if (action == Action::Quit) { m_running = false; return; }

        int newX = m_player.x;
        int newY = m_player.y;

        switch (action) {
        case Action::MoveUp:    --newY; break;
        case Action::MoveDown:  ++newY; break;
        case Action::MoveLeft:  --newX; break;
        case Action::MoveRight: ++newX; break;
        default: return;
        }

        if (newX < 0 || newX >= MAP_WIDTH || newY < 0 || newY >= MAP_HEIGHT)
            return;

        Enemy* enemy = getEnemyAt(newX, newY);
        if (enemy) {
            m_activeEnemy = enemy;
            m_state = GameState::Combat;
            m_log.clear();
            m_log.push_back("-- Combat: " + enemy->getName() + " --");
            m_log.push_back("Press Space to attack.");
            return;
        }

        if (isWalkable(newX, newY)) {
            m_player.x = newX;
            m_player.y = newY;
        }
    }

    void Game::handleCombat(Action action) {
        if (action == Action::Quit) { m_running = false; return; }
        if (action != Action::Interact) return;

        if (!m_activeEnemy) { endCombat(); return; }

        bool ongoing = m_combat.playerAttack(m_player, *m_activeEnemy, m_log);
        if (!ongoing) { endCombat(); return; }

        ongoing = m_combat.enemyAttack(m_player, *m_activeEnemy, m_log);
        if (!ongoing) {
            if (!m_player.isAlive())
                m_state = GameState::GameOver;
            else
                endCombat();
        }
    }

    void Game::endCombat() {
        if (m_activeEnemy && !m_activeEnemy->isAlive()) {
            m_dungeon.removeEntity(m_activeEnemy);
        }
        m_activeEnemy = nullptr;
        m_state = GameState::Exploring;
        m_log.clear();
        m_log.push_back("Victory!");
    }

    bool Game::isWalkable(int x, int y) const {
        if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
            return false;
        const auto& tile = m_dungeon.getGrid()[y][x];
        return tile.type == TileType::Floor;
    }

    Enemy* Game::getEnemyAt(int x, int y) const {
        for (const auto& e : m_dungeon.getEntities()) {
            if (e->isAlive() && e->getX() == x && e->getY() == y) {
                Enemy* enemy = dynamic_cast<Enemy*>(e.get());
                if (enemy) return enemy;
            }
        }
        return nullptr;
    }

}