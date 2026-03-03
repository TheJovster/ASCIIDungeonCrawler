#pragma once
#include "Player.h"
#include "Enemy.h"
#include <string>
#include <vector>

namespace DungeonGame {

    class CombatSystem {
    public:

        bool playerAttack(Player& player, Enemy& enemy, std::vector<std::string>& log);
        bool enemyAttack(Player& player, Enemy& enemy, std::vector<std::string>& log);

        bool isCombatOver(const Player& player, const Enemy& enemy) const;


    };
}