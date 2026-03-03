#include "CombatSystem.h"
#include <algorithm>
#include <random>

namespace DungeonGame {

    static std::mt19937 rng{ std::random_device{}() };

    static int randInt(int lo, int hi) {
        return std::uniform_int_distribution<int>(lo, hi)(rng);
    }

    static int calcDamage(int attack, int defense) {
        int base = std::max(1, attack - defense);
        int variance = randInt(-1, 2);
        return std::max(1, base + variance);
    }

    static bool isCrit() {
        return randInt(1, 10) == 1; // 10% crit chance
    }

    bool CombatSystem::playerAttack(Player& player, Enemy& enemy, std::vector<std::string>& log) {
        bool crit = isCrit();
        int  dmg = calcDamage(player.attack(), enemy.getDefense());
        if (crit) dmg = (int)(dmg * 1.5f);

        enemy.takeDamage(dmg);

        if (crit)
            log.push_back("Critical hit! " + std::to_string(dmg) + " damage to " + enemy.getName());
        else
            log.push_back("You hit " + enemy.getName() + " for " + std::to_string(dmg) + " damage.");

        if (!enemy.isAlive()) {
            log.push_back(enemy.getName() + " defeated! +" + std::to_string(enemy.getGoldDrop()) + "g");
            player.gold += enemy.getGoldDrop();
            return false;
        }
        return true;
    }

    bool CombatSystem::enemyAttack(Player& player, Enemy& enemy, std::vector<std::string>& log) {
        bool crit = isCrit();
        int  dmg = calcDamage(enemy.getAttack(), player.defense());
        if (crit) dmg = (int)(dmg * 1.5f);

        player.hp -= dmg;

        if (crit)
            log.push_back(enemy.getName() + " lands a critical hit for " + std::to_string(dmg) + "!");
        else
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