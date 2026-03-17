#define NOMINMAX
#include "Game.h"
#include "AudioManager.h"
#include <cmath>
#include <algorithm>

namespace DungeonGame {

    bool Game::enemiesNearby() const {
        for (const auto& e : m_dungeon.getEntities()) {
            if (!e->isAlive() || e->isCorpse()) continue;
            if (!e->isEnemy()) continue;
            int dist = std::abs(e->getX() - m_player.x) + std::abs(e->getY() - m_player.y);
            if (dist <= 5) return true;
        }
        return false;
    }

    void Game::handleRestMenu(Action action) {
        
        if (enemiesNearby()) {
            m_log.clear();
            m_log.push_back("Enemies are nearby. You cannot rest.");
            return;
        }

        if (action == Action::Quit) {
            m_state = GameState::Exploring;
            m_log.clear();
            return;
        }

        if (action == Action::MoveUp)
            m_restMenuSelected = std::max(0, m_restMenuSelected - 1);
        else if (action == Action::MoveDown)
            m_restMenuSelected = std::min(2, m_restMenuSelected + 1);
        else if (action == Action::Interact) {
            switch (m_restMenuSelected) {
            case 0: // Rest
                if (enemiesNearby()) {
                    m_log.clear();
                    m_log.push_back("Enemies are nearby. You cannot rest.");
                    return;
                }
                if (m_player.hp >= m_player.maxHP()) {
                    m_log.clear();
                    m_log.push_back("You are already at full health.");
                    return;
                }
                m_hoursRested = 0;
                m_restFade = 0.f;
                m_state = GameState::Resting;
                break;

            case 1: // Wait
                m_waitHours = 1;
                m_state = GameState::RestWaitSelect;
                break;

            case 2: // Leave
                m_state = GameState::Exploring;
                m_log.clear();
                break;
            }
        }
    }

    void Game::handleRestWaitSelect(Action action) {
        if (action == Action::Quit) {
            m_state = GameState::RestMenu;
            return;
        }

        if (action == Action::MoveUp)
            m_waitHours = std::min(12, m_waitHours + 1);
        else if (action == Action::MoveDown)
            m_waitHours = std::max(1, m_waitHours - 1);
        else if (action == Action::Interact) {
            if (enemiesNearby()) {
                m_log.clear();
                m_log.push_back("Enemies are nearby. You cannot wait.");
                m_state = GameState::RestMenu;
                return;
            }
            m_hoursRested = 0;
            m_restFade = 0.f;
            m_state = GameState::Waiting;
        }
    }

    void Game::handleResting(float dt) {
        // fade in
        m_restFade = std::min(1.f, m_restFade + dt * 2.f);
        if (m_restFade < 1.f) return;

        // accumulate time
        m_restHourTimer += dt;
        if (m_restHourTimer < REST_HOUR_DURATION) return;
        m_restHourTimer = 0.f;

        // check interrupt
        updateEnemyPatrol();
        if (enemiesNearby()) {
            m_log.clear();
            m_log.push_back("Your rest is interrupted by nearby enemies!");
            m_restFade = 0.f;
            m_state = GameState::Exploring;
            return;
        }

        // advance time and heal
        m_time.advanceHour();
        m_player.hp = std::min(m_player.maxHP(), m_player.hp + 6);
        m_hoursRested++;

        if (m_player.hp >= m_player.maxHP()) {
            m_log.clear();
            m_log.push_back("You wake up feeling refreshed. (" + std::to_string(m_hoursRested) + "h rested)");
            m_restFade = 0.f;
            m_state = GameState::Exploring;
        }
    }

    void Game::handleWaiting(float dt) {
        // fade in
        m_restFade = std::min(1.f, m_restFade + dt * 2.f);

        if (m_restFade < 1.f) return;

        // accumulate time
        m_restHourTimer += dt;
        if (m_restHourTimer < REST_HOUR_DURATION) return;
        m_restHourTimer = 0.f;

        // check interrupt
        updateEnemyPatrol();
        if (enemiesNearby()) {
            m_log.clear();
            m_log.push_back("Your wait is interrupted by nearby enemies!");
            m_restFade = 0.f;
            m_state = GameState::Exploring;
            return;
        }

        // advance time
        m_time.advanceHour();
        m_hoursRested++;

        // done waiting
        if (m_hoursRested >= m_waitHours) {
            m_log.clear();
            m_log.push_back("You waited " + std::to_string(m_hoursRested) + " hour(s).");
            m_restFade = 0.f;
            m_state = GameState::Exploring;
        }
    }
}