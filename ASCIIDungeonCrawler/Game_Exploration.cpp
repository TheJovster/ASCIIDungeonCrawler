#include "Game.h"
#include "AudioManager.h"
#include <random>

namespace DungeonGame 
{
    void Game::handleExploring(Action action) {
        if (action == Action::Quit) {
            m_state = GameState::QuitPrompt;
            return;
        }

        if (action == Action::ToggleInventory) {
            m_inventoryMode = !m_inventoryMode;
            return;
        }

        if (action == Action::Interact) {

            // enemy adjacent
            Enemy* enemy = getEnemyAdjacent(m_player.x, m_player.y);
            if (!m_inventoryMode &&
                enemy &&
                isPlayerFacing()) {
                m_activeEnemy = enemy;
                float dx = (float)(enemy->getX() - m_player.x);
                float dy = (float)(enemy->getY() - m_player.y);
                m_player.targetAngle = std::atan2(dy, dx);
                m_state = GameState::Combat;

                m_pendingEnemyTurn = false;
                m_pendingCombatEnd = false;

                m_combatPhase = CombatPhase::ActionSelect;
                m_combatActionSelected = 0;
                m_combat.clearLog();

                m_log.clear();
                return;
            }

            // chest adjacent
            int key = getChestKeyAdjacent(m_player.x, m_player.y);
            if (key != -1 &&
                !m_inventoryMode &&
                isPlayerFacing()) {
                m_chestKey = key;
                m_chestSelected = 0;
                m_state = GameState::ChestLoot;
                m_log.clear();
                return;
            }

            // merchant adjacent
            if (!m_inventoryMode &&
                isAdjacentToMerchant(m_player.x, m_player.y) &&
                isPlayerFacing()) {
                m_activeMerchant = getMerchantAt(
                    m_player.x + 0, m_player.y - 1); // check will find it
                // reuse existing adjacent scan
                int ddx[] = { 0, 0, -1, 1 };
                int ddy[] = { -1, 1, 0, 0 };
                for (int i = 0; i < 4; ++i) {
                    Merchant* m = getMerchantAt(
                        m_player.x + ddx[i], m_player.y + ddy[i]);
                    if (m) { m_activeMerchant = m; break; }
                }
                if (m_activeMerchant) {
                    m_merchantMode = MerchantMode::TopMenu;
                    m_merchantTopSelected = 0;
                    m_state = GameState::MerchantMenu;
                    m_log.clear();
                    return;
                }
            }

            // exit adjacent
            if (!m_inventoryMode &&
                isAdjacentToExit(m_player.x, m_player.y) &&
                isPlayerFacing()) {
                m_state = GameState::ExitPrompt;
                m_log.clear();
                return;
            }

            // inventory action
            if (m_inventoryMode && m_player.inventory.count() > 0) {
                m_inventoryActionSelected = 0;
                m_state = GameState::InventoryAction;
                return;
            }
            return;
        }

        // inventory scrolling mode
        if (m_inventoryMode) {
            if (action == Action::MoveUp)
                m_player.inventory.scrollUp();
            else if (action == Action::MoveDown)
                m_player.inventory.scrollDown();
            return;
        }

        if (action == Action::RotateLeft) {
            m_player.targetAngle -= PI * 0.5f;
            return;
        }
        if (action == Action::RotateRight) {
            m_player.targetAngle += PI * 0.5f;
            return;
        }

        // normal movement
        int newX = m_player.x;
        int newY = m_player.y;

        switch (action) {
        case Action::MoveUp:    // forward — move in facing direction
            newX = m_player.x + (int)std::round(std::cos(m_player.angle));
            newY = m_player.y + (int)std::round(std::sin(m_player.angle));
            break;
        case Action::MoveDown:  // backward
            newX = m_player.x - (int)std::round(std::cos(m_player.angle));
            newY = m_player.y - (int)std::round(std::sin(m_player.angle));
            break;
        case Action::MoveLeft:  // strafe left
            newX = m_player.x + (int)std::round(std::cos(m_player.angle - PI * 0.5f));
            newY = m_player.y + (int)std::round(std::sin(m_player.angle - PI * 0.5f));
            break;                                                       
        case Action::MoveRight: // strafe right                          
            newX = m_player.x + (int)std::round(std::cos(m_player.angle + PI * 0.5f));
            newY = m_player.y + (int)std::round(std::sin(m_player.angle + PI * 0.5f));
            break;
        default: return;
        }

        if (newX < 0 || newX >= MAP_WIDTH || newY < 0 || newY >= MAP_HEIGHT)
            return;

        if (isWalkable(newX, newY)) {
            m_player.x = newX;
            m_player.y = newY;
            // torch fuel decrement
            if (m_player.equipment.torch.has_value()) {
                m_player.equipment.torch->charges--;
                if (m_player.equipment.torch->charges <= 0) {
                    m_player.equipment.torch = std::nullopt;
                    m_log.clear();
                    m_log.push_back("Your torch burns out!");
                }
            }
            m_time.advanceMove();
            updateEnemyPatrol();
            updateVisibility();
        }
    }

    bool Game::isWalkable(int x, int y) const {
        if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) return false;
        const Tile& tile = m_dungeon.getGrid()[y][x];
        if (tile.type != TileType::Floor) return false;
        if (tile.isExit) return false;
        return true;
    }

    Enemy* Game::getEnemyAt(int x, int y) const {
        for (const auto& e : m_dungeon.getEntities()) {
            if (!e->isAlive() || !e->isEnemy()) continue;
            if (e->getX() == x && e->getY() == y)
                return static_cast<Enemy*>(e.get());
        }
        return nullptr;
    }

    Enemy* Game::getEnemyAdjacent(int x, int y) const {
        int dx[] = { 0, 0, -1, 1 };
        int dy[] = { -1, 1, 0, 0 };
        for (int i = 0; i < 4; ++i) {
            Enemy* e = getEnemyAt(x + dx[i], y + dy[i]);
            if (e) return e;
        }
        return nullptr;
    }

    bool Game::isAdjacentToExit(int x, int y) const {
        int dx[] = { 0, 0, -1, 1 };
        int dy[] = { -1, 1, 0, 0 };
        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i], ny = y + dy[i];
            if (nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT) continue;
            if (m_dungeon.getGrid()[ny][nx].isExit) return true;
        }
        return false;
    }

    bool Game::isPlayerFacing() const {
        // get tile directly in front of player based on facing angle
        int dx = (int)std::round(std::cos(m_player.angle));
        int dy = (int)std::round(std::sin(m_player.angle));
        int nx = m_player.x + dx;
        int ny = m_player.y + dy;

        if (nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT)
            return false;

        // enemy
        if (getEnemyAt(nx, ny)) return true;

        // chest
        if (getChestKeyAt(nx, ny) != -1) return true;

        // merchant
        if (getMerchantAt(nx, ny)) return true;

        // exit
        if (m_dungeon.getGrid()[ny][nx].isExit) return true;

        return false;
    }

    void Game::updateEnemyPatrol() {
        static std::mt19937 rng{ std::random_device{}() };

        const int dx[] = { 0, 0, -1, 1 };
        const int dy[] = { -1, 1, 0, 0 };

        m_pendingEnemyTurn = false;
        m_pendingCombatEnd = false;

        for (auto& e : m_dungeon.getEntities()) {
            if (!e->isAlive()) continue;
            if (!e->isEnemy()) continue;
            if (!e) continue;

            // collect valid moves
            std::vector<std::pair<int, int>> validMoves;
            for (int i = 0; i < 4; ++i) {
                int nx = e->getX() + dx[i];
                int ny = e->getY() + dy[i];

                if (nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT) continue;
                if (!isWalkable(nx, ny)) continue;

                // no chests
                int key = ny * MAP_WIDTH + nx;
                if (m_dungeon.getChests().count(key)) continue;

                // no merchants or other enemies — entities can pass each other
                // but don't land on a merchant
                bool hasMerchant = false;
                for (const auto& other : m_dungeon.getEntities()) {
                    if (!other->isAlive()) continue;
                    if (other.get() == e.get()) continue;
                    if (other->getX() == nx && other->getY() == ny) {
                        if (other->isMerchant()) { hasMerchant = true; break; }
                    }
                }
                if (hasMerchant) continue;

                validMoves.push_back({ nx, ny });
            }

            if (validMoves.empty()) continue;

            // pick random valid move
            auto [nx, ny] = validMoves[std::uniform_int_distribution<int>(0, (int)validMoves.size() - 1)(rng)];

            // check if moving into player — trigger combat
            if (nx == m_player.x && ny == m_player.y) {
                m_activeEnemy = static_cast<Enemy*>(e.get());
                float dx = (float)(e->getX() - m_player.x);
                float dy = (float)(e->getY() - m_player.y);
                if (!isPlayerFacing())
                    m_player.targetAngle = std::atan2(dy, dx);
                m_state = GameState::Combat;
                m_combatPhase = CombatPhase::Ambush;
                m_combatActionSelected = 0;
                m_combat.clearLog();
                bool playerAlive = m_combat.enemyTurn(m_player, *m_activeEnemy);
                m_raycastRenderer.triggerHitFlash();
                if (m_combat.wasLastHitCrit())
                    m_raycastRenderer.triggerCritFlash();
                if (!playerAlive) {
                    AudioManager::get().playMusic(MusicTrack::GameOver);
                    m_state = GameState::GameOver;
                }
                m_log.clear();
                return;
            }

            e->setPosition(nx, ny);
        }
    }

}