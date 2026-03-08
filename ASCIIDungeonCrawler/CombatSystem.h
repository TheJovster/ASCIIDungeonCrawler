#pragma once
#include "Player.h"
#include "Enemy.h"
#include <string>
#include <vector>
#include <optional>

namespace DungeonGame {

    enum class CombatPhase {
        ActionSelect,
        ItemSelect,
        Resolution
    };

    enum class CombatAction {
        Attack,
        Defend,
        UseItem,
        Flee
    };

    class CombatSystem {
    public:
        // --- turn resolution ---
        bool playerAttack(Player& player, Enemy& enemy);
        bool playerDefend(Player& player, Enemy& enemy);
        bool playerUseItem(Player& player, Enemy& enemy, int itemIndex);
        bool playerFlee(Player& player, Enemy& enemy);

        // called after player action — always happens unless combat ended
        bool enemyTurn(Player& player, Enemy& enemy);
        bool wasLastHitCrit() const { return m_lastHitWasCrit; }

        bool isCombatOver(const Player& player, const Enemy& enemy) const;

        // --- state ---
        void clearLog();
        void clearDefend();  // call at start of each player turn

        // --- log accessors ---
        const std::string& getLastAction() const { return m_lastAction; }
        const std::vector<std::string>& getHistory()    const { return m_history; }

    private:
        bool m_playerDefending = false;
        bool m_enemyDefending = false; // unused now, ready for later
        bool m_lastHitWasCrit = false;

        std::string              m_lastAction;
        std::vector<std::string> m_history;

        void pushLog(const std::string& msg);
    };
}