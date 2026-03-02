#include "CombatSystem.h"
#include <algorithm>

namespace DungeonGame {

    static int calcDamage(int attack, int defense) {
        return std::max(1, attack - defense);
    }

    bool CombatSystem::playerAttack(Player& player, Enemy& enemy, std::vector<std::string>& log) {
        int dmg = calcDamage(player.attack(), enemy.getDefense());
        enemy.takeDamage(dmg);
        log.push_back("You hit " + enemy.getName() + " for " + std::to_string(dmg) + " damage.");

        if (!enemy.isAlive()) {
            log.push_back(enemy.getName() + " is defeated! You gain " + std::to_string(enemy.getGoldDrop()) + " gold.");
            player.gold += enemy.getGoldDrop();
            return false;
        }
        return true; 
    }

    bool CombatSystem::enemyAttack(Player& player, Enemy& enemy, std::vector<std::string>& log) {
        int dmg = calcDamage(enemy.getAttack(), player.defense());
        player.hp -= dmg;
        log.push_back(enemy.getName() + " hits you for " + std::to_string(dmg) + " damage.");

        if (!player.isAlive()) {
            log.push_back("You have been defeated...");
            return false;
        }
        return true; 
    }

    bool CombatSystem::isCombatOver(const Player& player, const Enemy& enemy) const {
        return !player.isAlive() || !enemy.isAlive();
    }

}