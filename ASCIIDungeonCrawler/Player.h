#pragma once
#include <string>

namespace DungeonGame {

    struct Player {
        int x = 0;
        int y = 0;

        // stats
        int hp = 30;
        int maxHP = 30;
        int attack = 5;
        int defense = 2;
        int gold = 0;

        bool isAlive() const { return hp > 0; }
    };

}