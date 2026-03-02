#pragma once
#include "Dungeon.h"
#include "Renderer.h"
#include "Player.h"
#include "Input.h"
#include "CombatSystem.h"
#include <vector>
#include <string>

namespace DungeonGame {

    enum class GameState {
        MainMenu,
        Exploring,
        Combat,
        GameOver
    };

    class Game {
    public:
        Game();
        void run();

    private:
        Dungeon      m_dungeon;
        Renderer     m_renderer;
        Player       m_player;
        CombatSystem m_combat;
        GameState    m_state = GameState::Exploring;
        Enemy* m_activeEnemy = nullptr;
        bool         m_running = true;
        bool m_inventoryMode = false;
        int m_floor = 1;

        std::vector<std::string> m_log;

        void spawnPlayer();
        void handleExploring(Action action);
        void handleCombat(Action action);
        void endCombat();
        bool isWalkable(int x, int y) const;
        Enemy* getEnemyAt(int x, int y) const;
    };

}