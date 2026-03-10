#include "CombatSystem.h"
#include <algorithm>
#include <random>

namespace DungeonGame {

    static std::mt19937 rng{ std::random_device{}() };

    static int randInt(int lo, int hi) {
        return std::uniform_int_distribution<int>(lo, hi)(rng);
    }

    static int calcDamage(int attack, int defense, bool defenseActive) {
        float reduction = std::min(0.75f, defense / (float)(defense + 50));
        if (defenseActive) reduction = std::min(0.90f, reduction + 0.30f);
        int base = std::max(1, (int)(attack * (1.0f - reduction)));
        int variance = randInt(-1, 2);
        return std::max(1, base + variance);
    }

    static bool isCrit() {
        return randInt(1, 10) == 1;
    }

    void CombatSystem::pushLog(const std::string& msg) {
        if (!m_lastAction.empty())
            m_history.push_back(m_lastAction);
        m_lastAction = msg;
        // cap history at 6 lines
        if (m_history.size() > 6)
            m_history.erase(m_history.begin());
    }

    void CombatSystem::clearLog() {
        m_lastAction.clear();
        m_history.clear();
        m_lastHitWasCrit = false;

    }

    void CombatSystem::clearDefend() {
        m_playerDefending = false;
        m_enemyDefending = false;
    }

    bool CombatSystem::playerAttack(Player& player, Enemy& enemy) {
        bool crit = isCrit();
        m_lastHitWasCrit = crit;
        int  dmg = calcDamage(player.attack(), enemy.getDefense(), false);
        if (crit) dmg = (int)(dmg * 1.5f);

        enemy.getAnimator().setState(AnimationState::Hit);
        enemy.getAnimator().setReturnState(AnimationState::IdlePassive);
        enemy.takeDamage(dmg);

        if (crit)
            pushLog("Critical hit! " + std::to_string(dmg) + " damage to " + enemy.getName() + "!");
        else
            pushLog("You hit " + enemy.getName() + " for " + std::to_string(dmg) + " damage.");

        return enemy.isAlive();
    }

    bool CombatSystem::playerDefend(Player& player, Enemy& enemy) {
        m_playerDefending = true;
        pushLog("You brace for impact. Incoming damage reduced.");
        return true; // combat continues
    }

    bool CombatSystem::playerUseItem(Player& player, Enemy& enemy, int itemIndex) {
        auto& inv = player.inventory;
        if (itemIndex < 0 || itemIndex >= inv.count()) {
            pushLog("Nothing to use.");
            return true;
        }

        const Item& item = inv.getItem(itemIndex);
        if (item.type != ItemType::Consumable) {
            pushLog("Can't use that in combat.");
            return true;
        }

        // apply heal
        int healAmt = item.healAmount;
        player.hp = std::min(player.maxHP(), player.hp + healAmt);
        pushLog("Used " + item.name + ". Restored " + std::to_string(healAmt) + " HP.");

        inv.removeItem(itemIndex);
        return true;
    }

    bool CombatSystem::playerFlee(Player& player, Enemy& enemy) {
        // flee chance by tier: Basic=60%, Agile=40%, Heavy=20%
        int roll = randInt(1, 100);
        int threshold = 0;
        switch (enemy.getTier()) {
        case EnemyTier::Basic:  threshold = 60; break;
        case EnemyTier::Agile:  threshold = 40; break;
        case EnemyTier::Heavy:  threshold = 20; break;
        }

        if (roll <= threshold) {
            pushLog("You escaped!");
            return false; // signals flee success — Game handles state transition
        }
        else {
            pushLog("Failed to flee!");
            return true; // combat continues — enemy gets their turn
        }
    }

    bool CombatSystem::enemyTurn(Player& player, Enemy& enemy) {
        bool crit = isCrit();
        m_lastHitWasCrit = crit;
        int  dmg = calcDamage(enemy.getAttack(), player.defense(), m_playerDefending);
        if (crit) dmg = (int)(dmg * 1.5f);

        enemy.getAnimator().setState(AnimationState::Attack);
        enemy.getAnimator().setReturnState(AnimationState::IdlePassive);
        player.hp -= dmg;

        if (crit)
            pushLog(enemy.getName() + " lands a critical hit for " + std::to_string(dmg) + "!");
        else
            pushLog(enemy.getName() + " hits you for " + std::to_string(dmg) + ".");

        if (!player.isAlive()) {
            pushLog("You have been defeated...");
            return false;
        }
        return true;
    }

    bool CombatSystem::isCombatOver(const Player& player, const Enemy& enemy) const {
        return !player.isAlive() || !enemy.isAlive();
    }
}