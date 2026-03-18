#define NOMINMAX
#include "Game.h"
#include "InventoryItem.h"
#include "Merchant.h"
#include <windows.h>
#include <iostream>
#include <algorithm>

namespace DungeonGame {

    Game::Game(sf::RenderWindow& window)
        : m_renderer(window), m_raycastRenderer(window) {
        std::ios::sync_with_stdio(false);
        std::cin.tie(nullptr);
        m_dungeon.generate(m_floor);
        spawnPlayer();
        
        AudioManager::get().playMusic(MusicTrack::Game);
    }

    void Game::run(sf::RenderWindow& window) {
        sf::Clock clock;

        while (m_running && window.isOpen()) {

            CombatHUDData combatData;
            combatData.phase = m_combatPhase;
            combatData.actionSelected = m_combatActionSelected;
            combatData.itemSelected = m_combatItemSelected;
            combatData.itemList = &m_combatItemList;
            combatData.playerInventory = &m_player.inventory; 
            combatData.lastAction = &m_combat.getLastAction();
            combatData.history = &m_combat.getHistory();

            RestHUDData restData;
            restData.menuSelected = m_restMenuSelected;
            restData.waitHours = m_waitHours;
            restData.hoursRested = m_hoursRested;
            restData.message = m_log.empty() ? "" : m_log[0];

            // smooth rotation lerp
            float lerpSpeed = 10.f;
            float dt = clock.restart().asSeconds();

            //rest message timer
            if (m_restMessageTimer > 0.f) {
                m_restMessageTimer -= dt;
                if (m_restMessageTimer <= 0.f) {
                    m_restMessageTimer = 0.f;
                    if (m_state == GameState::RestMenu)
                        m_state = GameState::Exploring;
                }
            }

            // rest fade
            if (m_restFadingOut) {
                m_restFade = std::max(0.f, m_restFade - dt * 2.f);
                if (m_restFade <= 0.f) {
                    m_restFadingOut = false;
                    m_state = GameState::Exploring;
                }
            }

            float diff = m_player.targetAngle - m_player.angle;
            m_player.angle += diff * 10.f * dt;
            float posDiffX = (float)m_player.x - m_player.visualX;
            float posDiffY = (float)m_player.y - m_player.visualY;
            m_player.visualX += posDiffX * 10.f * dt;
            m_player.visualY += posDiffY * 10.f * dt;

            for (auto& e : m_dungeon.getEntities()) {
                if (!e->isAlive() && !e->isCorpse()) continue;
                float edx = (float)e->getX() - e->visualX;
                float edy = (float)e->getY() - e->visualY;
                e->visualX += edx * 10.f * dt;
                e->visualY += edy * 10.f * dt;
                e->update(dt);
            }

            sf::Event event;
            sf::Joystick::update();
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                    return;
                }
                Action action = getInput(event);

                switch (m_state) {
                case GameState::Exploring:       handleExploring(action);       break;
                case GameState::Combat:          handleCombat(action);          break;
                case GameState::ChestLoot:       handleChestLoot(action);       break;
                case GameState::InventoryAction: handleInventoryAction(action); break;
                case GameState::MerchantMenu:    handleMerchantMenu(action);    break;
                case GameState::GameOver:        handleGameOver(action);        break;
                case GameState::QuitPrompt:      handleQuitPrompt(action);      break;
                case GameState::ExitPrompt:      handleExitPrompt(action);      break;
                case GameState::RestMenu:        handleRestMenu(action);        break;
                case GameState::RestWaitSelect:  handleRestWaitSelect(action);  break;
                case GameState::Resting:                                        break;
                case GameState::Waiting:                                        break;
                }
                break;
            }

            // frame-driven states
            if (m_state == GameState::Resting && !m_restFadingOut) handleResting(dt);
            if (m_state == GameState::Waiting && !m_restFadingOut) handleWaiting(dt);

            window.clear(sf::Color::Black);
            m_raycastRenderer.draw(window, m_dungeon, m_player, dt, m_restFade);
            m_raycastRenderer.drawMinimap(window, m_dungeon, m_player);

            // chestContents block unchanged
            const std::vector<Item>* chestContents = nullptr;
            if (m_state == GameState::ChestLoot && m_chestKey != -1) {
                auto it = m_dungeon.getChests().find(m_chestKey);
                if (it != m_dungeon.getChests().end())
                    chestContents = &it->second;
            }

            m_renderer.drawHUD(window, m_player, m_state, m_activeEnemy, m_floor,
                m_inventoryMode, chestContents, m_chestSelected,
                m_inventoryActionSelected, m_activeMerchant,
                m_merchantMode, m_merchantTopSelected, m_sellIndex,
                combatData, restData,
                m_time.getTimeString() + " " + m_time.getDayOfWeek() + " " + m_time.getDateString());
            window.display();
        }
    }

    void Game::updateVisibility() {
        auto& grid = m_dungeon.getGrid();

        // clear visible every move — visited persists
        for (int y = 0; y < MAP_HEIGHT; ++y)
            for (int x = 0; x < MAP_WIDTH; ++x)
                grid[y][x].visible = false;

        // player's current tile — always visited
        grid[m_player.y][m_player.x].visited = true;
        grid[m_player.y][m_player.x].visible = true;

        bool hasTorch = m_player.equipment.torch.has_value();
        int radius = hasTorch ? 6 : 1;

        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                if (dx * dx + dy * dy > radius * radius) continue;
                int nx = m_player.x + dx;
                int ny = m_player.y + dy;
                if (nx < 0 || nx >= MAP_WIDTH) continue;
                if (ny < 0 || ny >= MAP_HEIGHT) continue;
                grid[ny][nx].visible = true;

                // permanently mark exit
                if (grid[ny][nx].isExit)
                    grid[ny][nx].visited = true;

                //permanently mark entities
                for (const auto& e : m_dungeon.getEntities())
                    if (e->isAlive() && e->getX() == nx && e->getY() == ny)
                        grid[ny][nx].visited = true;
            }
        }
    }

    void Game::spawnPlayer() {
        const Room& first = m_dungeon.getRooms()[0];
        m_player.x = first.centerX();
        m_player.y = first.centerY();
        m_player.visualX = (float)m_player.x;
        m_player.visualY = (float)m_player.y;

        // give player a starting torch 
        // impossible to play without this
        Item startTorch;
        startTorch.id = 0;
        startTorch.name = "Torch";
        startTorch.type = ItemType::Equipment;
        startTorch.slot = EquipSlot::Torch;
        startTorch.charges = 100;
        startTorch.value = 5;
        m_player.equipment.torch = startTorch;

        updateVisibility();
    }


}
