#pragma once
#include "Dungeon.h"
#include "Player.h"
#include <vector>
#include <string>

namespace DungeonGame {

    enum class GameState;  // forward declare — defined in Game.h

    class Renderer {
    public:
        void drawMap(const Dungeon& dungeon, const Player& player,
            const std::vector<std::string>& log,
            GameState state) const;
        void drawHUD(const Player& player, GameState state,
            const Enemy* activeEnemy, int floor,
            bool inventoryMode) const;
    };

}