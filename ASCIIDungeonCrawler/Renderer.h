#pragma once
#include "Dungeon.h"
#include "Player.h"

namespace DungeonGame {

    class Renderer {
    public:
        void drawMap(const Dungeon& dungeon, const Player& player) const;
    };

}