#include "Game.h"
#include "AudioManager.h"
#include "CombatSystem.h"


namespace DungeonGame 
{
    void Game::handleCombat(Action action) {
        if (action == Action::Quit) {
            if (m_combatPhase == CombatPhase::ItemSelect) {
                m_combatPhase = CombatPhase::ActionSelect;
                return;
            }
            m_running = false;
            return;
        }

        switch (m_combatPhase) {

        case CombatPhase::ActionSelect:
            if (action == Action::MoveUp) {
                m_combatActionSelected = (m_combatActionSelected + 3) % 4;
            }
            else if (action == Action::MoveDown) {
                m_combatActionSelected = (m_combatActionSelected + 1) % 4;
            }
            else if (action == Action::Interact) {
                switch (m_combatActionSelected) {
                case 0: // Attack
                    m_combat.clearDefend();
                    m_raycastRenderer.triggerCritFlash();
                    resolveCombatTurn(CombatAction::Attack);
                    break;
                case 1: // Defend
                    m_combat.clearDefend();
                    resolveCombatTurn(CombatAction::Defend);
                    break;
                case 2: // Use Item — switch to item select if consumables exist
                    m_combatItemList = buildCombatItemList();
                    if (m_combatItemList.empty()) {
                        // no consumables — stay on action select, log it
                        m_combat.clearLog();
                        // reuse pushLog indirectly — just set a note
                    }
                    else {
                        m_combatItemSelected = 0;
                        m_combatPhase = CombatPhase::ItemSelect;
                    }
                    break;
                case 3: // Flee
                    m_combat.clearDefend();
                    resolveCombatTurn(CombatAction::Flee);
                    break;
                }
            }
            break;

        case CombatPhase::ItemSelect:
            if (action == Action::MoveUp) {
                m_combatItemSelected = (m_combatItemSelected + (int)m_combatItemList.size() - 1)
                    % (int)m_combatItemList.size();
            }
            else if (action == Action::MoveDown) {
                m_combatItemSelected = (m_combatItemSelected + 1) % (int)m_combatItemList.size();
            }
            else if (action == Action::Interact) {
                m_combat.clearDefend();
                resolveCombatTurn(CombatAction::UseItem);
            }
            else if (action == Action::Back) {
                m_combatPhase = CombatPhase::ActionSelect;
            }
            break;

        case CombatPhase::Ambush:
            if (action == Action::MoveUp)
                m_combatActionSelected = 0;
            else if (action == Action::MoveDown)
                m_combatActionSelected = 1;
            else if (action == Action::Interact) {
                if (m_combatActionSelected == 0) {
                    // Fight — go straight to ActionSelect, no free hit
                    m_combatPhase = CombatPhase::ActionSelect;
                    m_combatActionSelected = 0;
                    m_pendingEnemyTurn = false;
                    m_pendingCombatEnd = false;
                }
                else {
                    // Flee attempt
                    bool fled = !m_combat.playerFlee(m_player, *m_activeEnemy);
                    if (fled) {
                        m_activeEnemy = nullptr;
                        m_state = GameState::Exploring;
                        m_combat.clearLog();
                        m_log.push_back("You backed away.");
                    }
                    else {
                        // Failed to flee — now enemy gets their hit as punishment
                        m_combatPhase = CombatPhase::Resolution;
                        m_pendingEnemyTurn = true;
                        m_pendingCombatEnd = false;
                    }
                }
            }
            break;

        case CombatPhase::Resolution:
            if (action == Action::Interact) {
                if (m_pendingCombatEnd) {
                    endCombat();
                    return;
                }
                if (m_pendingEnemyTurn) {
                    m_pendingEnemyTurn = false;
                    bool playerAlive = m_combat.enemyTurn(m_player, *m_activeEnemy);
                    m_raycastRenderer.triggerHitFlash();
                    if (m_combat.wasLastHitCrit())
                        m_raycastRenderer.triggerCritFlash();
                    if (!playerAlive) {
                        AudioManager::get().playMusic(MusicTrack::GameOver);
                        m_state = GameState::GameOver;
                        return;
                    }
                    //stay on resolution - player sees enemy results next
                }
                else {
                    m_combatPhase = CombatPhase::ActionSelect;
                }
            }
            break;
        }
    }

    void Game::resolveCombatTurn(CombatAction combatAction) {
        bool combatContinues = true;
        m_time.advanceTurn();

        switch (combatAction) {
        case CombatAction::Attack:
            combatContinues = m_combat.playerAttack(m_player, *m_activeEnemy);
            if (m_combat.wasLastHitCrit())
                m_raycastRenderer.triggerCritFlash();
            break;
        case CombatAction::Defend:
            combatContinues = m_combat.playerDefend(m_player, *m_activeEnemy);
            break;
        case CombatAction::UseItem:
            combatContinues = m_combat.playerUseItem(m_player, *m_activeEnemy,
                m_combatItemList[m_combatItemSelected]);
            break;
        case CombatAction::Flee:
            combatContinues = m_combat.playerFlee(m_player, *m_activeEnemy);
            if (!combatContinues) {
                // fled successfully — show message then end
                m_combatPhase = CombatPhase::Resolution;
                m_pendingEnemyTurn = false;
                m_pendingCombatEnd = true;
                return;
            }
            break;
        }

        if (!combatContinues) {
            // enemy dead — show kill message then end
            m_combatPhase = CombatPhase::Resolution;
            m_pendingEnemyTurn = false;
            m_pendingCombatEnd = true;
            return;
        }

        // player action landed, enemy still alive — show result, enemy fires next
        m_combatPhase = CombatPhase::Resolution;
        m_pendingEnemyTurn = true;
        m_pendingCombatEnd = false;
    }

    void Game::endCombat() {
        m_activeEnemy = nullptr;
        m_state = GameState::Exploring;
        m_log.clear();
        m_log.push_back("Victory!");
    }

    std::vector<int> Game::buildCombatItemList() {
        std::vector<int> result;
        for (int i = 0; i < m_player.inventory.count(); ++i) {
            if (m_player.inventory.getItem(i).type == ItemType::Consumable)
                result.push_back(i);
        }
        return result;
    }
}