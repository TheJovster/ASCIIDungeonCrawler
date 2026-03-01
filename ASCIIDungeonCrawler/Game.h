#pragma once
#include "Dungeon.h"
#include "Renderer.h"
#include "Player.h"
#include "Input.h"

namespace DungeonGame {

    class Game {
    public:
        Game();
        void run();

    private:
        Dungeon  m_dungeon;
        Renderer m_renderer;
        Player   m_player;
        bool     m_running = true;

        void spawnPlayer();
        void handleAction(Action action);
        bool isWalkable(int x, int y) const;
    };

}