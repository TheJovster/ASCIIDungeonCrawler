#define NOMINMAX
#include "Game.h"
#include "InventoryItem.h"
#include "Merchant.h"
#include <windows.h>
#include <iostream>
#include <algorithm>
#include <random>

namespace DungeonGame {

    Game::Game() {
        std::ios::sync_with_stdio(false);
        std::cin.tie(nullptr);
        m_dungeon.generate(m_floor);
        spawnPlayer();
    }

    void Game::run() {
        while (m_running) {
            m_renderer.drawMap(m_dungeon, m_player, m_log, m_state);

            const std::vector<Item>* chestContents = nullptr;
            if (m_state == GameState::ChestLoot && m_chestKey != -1) {
                auto it = m_dungeon.getChests().find(m_chestKey);
                if (it != m_dungeon.getChests().end())
                    chestContents = &it->second; 
            }

            m_renderer.drawHUD(m_player, m_state, m_activeEnemy, m_floor,
                m_inventoryMode, chestContents, m_chestSelected,
                m_inventoryActionSelected, m_activeMerchant,
                m_merchantMode, m_merchantTopSelected, m_sellIndex);

            Action action = getInput();

            switch (m_state) {
            case GameState::Exploring:       handleExploring(action);       break;
            case GameState::Combat:          handleCombat(action);          break;
            case GameState::ChestLoot:       handleChestLoot(action);       break;
            case GameState::InventoryAction: handleInventoryAction(action); break;
            case GameState::MerchantMenu:    handleMerchantMenu(action);    break; 
            case GameState::GameOver:        m_running = false;             break;
            case GameState::QuitPrompt:      handleQuitPrompt(action);      break;
            case GameState::ExitPrompt:      handleExitPrompt(action);      break;
            }
        }
    }

    void Game::spawnPlayer() {
        const Room& first = m_dungeon.getRooms()[0];
        m_player.x = first.centerX();
        m_player.y = first.centerY();
    }

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
            // chest on current tile
            if (!m_inventoryMode)
            {
                int key = getChestKeyAt(m_player.x, m_player.y);
                if (key != -1) {
                    m_chestKey = key;
                    m_chestSelected = 0;
                    m_state = GameState::ChestLoot;
                    m_log.clear();
                    return;
                }
            }

            // exit tile check
            const Tile& currentTile = m_dungeon.getGrid()[m_player.y][m_player.x];
            if (currentTile.isExit) {
                m_state = GameState::ExitPrompt;
                m_log.clear();
                return;
            }

            // adjacent merchant
            if (!m_inventoryMode)
            {
                if (isAdjacentToMerchant(m_player.x, m_player.y)) {
                    // find which merchant
                    int dx[] = { 0, 0, -1, 1 };
                    int dy[] = { -1, 1, 0, 0 };
                    for (int i = 0; i < 4; ++i) {
                        Merchant* m = getMerchantAt(m_player.x + dx[i], m_player.y + dy[i]);
                        if (m) {
                            m_activeMerchant = m;
                            m_merchantMode = MerchantMode::TopMenu;
                            m_merchantTopSelected = 0;
                            m_state = GameState::MerchantMenu;
                            m_log.clear();
                            return;
                        }
                    }
                }
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

        // normal movement
        int newX = m_player.x;
        int newY = m_player.y;

        switch (action) {
        case Action::MoveUp:    --newY; break;
        case Action::MoveDown:  ++newY; break;
        case Action::MoveLeft:  --newX; break;
        case Action::MoveRight: ++newX; break;
        default: return;
        }

        if (newX < 0 || newX >= MAP_WIDTH || newY < 0 || newY >= MAP_HEIGHT)
            return;

        Enemy* enemy = getEnemyAt(newX, newY);
        if (enemy) {
            m_activeEnemy = enemy;
            m_state = GameState::Combat;
            m_log.clear();
            m_log.push_back("-- Combat: " + enemy->getName() + " --");
            m_log.push_back("Press Space to attack.");
            return;
        }

        // check for merchant on target tile — block movement, same as enemy
        Merchant* merchant = getMerchantAt(newX, newY);
        if (merchant) {
            // blocked — can't walk into merchant
            return;
        }

        if (isWalkable(newX, newY)) {
            m_player.x = newX;
            m_player.y = newY;
            updateEnemyPatrol();
        }
    }

    void Game::handleCombat(Action action) {
        if (action == Action::Quit) { m_running = false; return; }
        if (action != Action::Interact) return;

        if (!m_activeEnemy) { endCombat(); return; }

        bool ongoing = m_combat.playerAttack(m_player, *m_activeEnemy, m_log);
        if (!ongoing) { endCombat(); return; }

        ongoing = m_combat.enemyAttack(m_player, *m_activeEnemy, m_log);
        if (!ongoing) {
            if (!m_player.isAlive())
                m_state = GameState::GameOver;
            else
                endCombat();
        }
    }

    void Game::endCombat() {
        if (m_activeEnemy && !m_activeEnemy->isAlive()) {
            m_dungeon.removeEntity(m_activeEnemy);
        }
        m_activeEnemy = nullptr;
        m_state = GameState::Exploring;
        m_log.clear();
        m_log.push_back("Victory!");
    }

    bool Game::isWalkable(int x, int y) const {
        if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
            return false;
        const auto& tile = m_dungeon.getGrid()[y][x];
        return tile.type == TileType::Floor;
    }

    Enemy* Game::getEnemyAt(int x, int y) const {
        for (const auto& e : m_dungeon.getEntities()) {
            if (e->isAlive() && e->getX() == x && e->getY() == y) {
                Enemy* enemy = dynamic_cast<Enemy*>(e.get());
                if (enemy) return enemy;
            }
        }
        return nullptr;
    }

    int Game::getChestKeyAt(int x, int y) const {
        int key = y * MAP_WIDTH + x;
        const auto& chests = m_dungeon.getChests();
        auto it = chests.find(key);
        if (it != chests.end()) return key;
        return -1;
    }

    void Game::handleChestLoot(Action action) {
        if (action == Action::Quit) {
            m_chestKey = -1;
            m_chestSelected = 0;
            m_state = GameState::Exploring;
            m_log.clear();
            return;
        }

        auto& chests = m_dungeon.getChests();
        auto  it = chests.find(m_chestKey);
        if (it == chests.end()) {
            m_state = GameState::Exploring;
            return;
        }

        auto& contents = it->second;

        if (action == Action::MoveUp) {
            if (m_chestSelected > 0) --m_chestSelected;
            return;
        }
        if (action == Action::MoveDown) {
            if (m_chestSelected < (int)contents.size() - 1) ++m_chestSelected;
            return;
        }

        if (action == Action::Interact) {
            if (contents.empty()) {
                m_state = GameState::Exploring;
                return;
            }
            if (m_player.inventory.isFull()) {
                m_log.clear();
                m_log.push_back("Inventory full!");
                return;
            }

            Item taken = contents[m_chestSelected];
            m_player.inventory.addItem(taken);
            contents.erase(contents.begin() + m_chestSelected);

            if (m_chestSelected >= (int)contents.size() && m_chestSelected > 0)
                --m_chestSelected;

            m_log.clear();
            m_log.push_back("Took: " + taken.name);

            if (contents.empty()) {
                m_log.push_back("Chest is empty.");
                m_state = GameState::Exploring;
            }
            return;
        }
    }

    void Game::handleInventoryAction(Action action) {
        if (action == Action::Quit) {
            m_state = GameState::Exploring;
            return;
        }

        m_log.clear();
        m_log.push_back("Action: " + std::to_string((int)action) +
            " State: " + std::to_string((int)m_state) +
            " Sel: " + std::to_string(m_inventoryActionSelected));
        

        const Item& item = m_player.inventory.getItem(m_player.inventory.getSelectedIndex());

        // build valid actions for this item
        // 0 = Equip (equipment only), 0 or 1 = Use (consumable only), last = Drop
        bool isEquipment = item.type == ItemType::Equipment;
        bool isConsumable = item.type == ItemType::Consumable;

        // options: equipment → [Equip, Drop]
        //          consumable → [Use, Drop]
        //          vendor trash → [Drop]
        int optionCount = (isEquipment || isConsumable) ? 2 : 1;

        if (action == Action::MoveUp) {
            if (m_inventoryActionSelected > 0) --m_inventoryActionSelected;
            return;
        }
        if (action == Action::MoveDown) {
            if (m_inventoryActionSelected < optionCount - 1) ++m_inventoryActionSelected;
            return;
        }

        if (action == Action::Interact) {
            if (isEquipment && m_inventoryActionSelected == 0) {
                // equip — swap with currently equipped item if any
                std::optional<Item> previous = m_player.equipment.equip(item);
                m_player.inventory.removeItem(m_player.inventory.getSelectedIndex());
                if (previous.has_value())
                    m_player.inventory.addItem(previous.value());
                // recalculate hp cap — don't let hp exceed new maxHP
                if (m_player.hp > m_player.maxHP())
                    m_player.hp = m_player.maxHP();
                m_log.clear();
                m_log.push_back("Equipped: " + item.name);
                /*m_state = GameState::Exploring;*/ //right now, I think it's better for the player the control the flow of state via tab
            }
            else if (isConsumable && m_inventoryActionSelected == 0) {
                // use — heal
                m_player.hp = std::min(m_player.hp + item.healAmount, m_player.maxHP());
                m_player.inventory.removeItem(m_player.inventory.getSelectedIndex());
                m_log.clear();
                m_log.push_back("Used: " + item.name + " (+" + std::to_string(item.healAmount) + " HP)");
                updateEnemyPatrol();  // using an item costs a turn
                m_state = GameState::Exploring;
            }
            else {
                // drop — last option for all item types
                m_log.clear();
                m_log.push_back("Dropped: " + item.name);
                m_player.inventory.removeItem(m_player.inventory.getSelectedIndex());
            }

            m_state = GameState::Exploring;
            return;
        }
    }

    Merchant* Game::getMerchantAt(int x, int y) const {
        for (const auto& e : m_dungeon.getEntities()) {
            if (e->isAlive() && e->getX() == x && e->getY() == y) {
                Merchant* m = dynamic_cast<Merchant*>(e.get());
                if (m) return m;
            }
        }
        return nullptr;
    }

    bool Game::isAdjacentToMerchant(int x, int y) const {
        // check all four adjacent tiles
        int dx[] = { 0, 0, -1, 1 };
        int dy[] = { -1, 1, 0, 0 };
        for (int i = 0; i < 4; ++i) {
            if (getMerchantAt(x + dx[i], y + dy[i]))
                return true;
        }
        return false;
    }

    void Game::handleMerchantMenu(Action action) {
        if (!m_activeMerchant) { m_state = GameState::Exploring; return; }

        if (action == Action::Quit) {
            // esc backs out one level
            if (m_merchantMode == MerchantMode::TopMenu) {
                m_activeMerchant = nullptr;
                m_state = GameState::Exploring;
                m_log.clear();
            }
            else {
                m_merchantMode = MerchantMode::TopMenu;
                m_merchantTopSelected = 0;
                m_activeMerchant->setSelectedIndex(0);
            }
            return;
        }

        switch (m_merchantMode) {

        case MerchantMode::TopMenu: {
            if (action == Action::MoveUp)
                m_merchantTopSelected = std::max(0, m_merchantTopSelected - 1);
            else if (action == Action::MoveDown)
                m_merchantTopSelected = std::min(3, m_merchantTopSelected + 1);
            else if (action == Action::Interact) {
                if (m_merchantTopSelected == 0) {
                    m_merchantMode = MerchantMode::Buy;
                    m_activeMerchant->setSelectedIndex(0);
                }
                else if (m_merchantTopSelected == 1) {
                    m_merchantMode = MerchantMode::Sell;
                    m_player.inventory.resetScroll(); // reset scroll to top
                }
                else if (m_merchantTopSelected == 2) {
                    // Upgrade inventory
                    int cost = 150 * (1 << ((m_player.inventory.capacity() - 30) / 10));
                    if (m_player.gold < cost) {
                        m_log.clear();
                        m_log.push_back("Need " + std::to_string(cost) + "g to upgrade.");
                    }
                    else if (m_player.inventory.capacity() >= 80) {
                        m_log.clear();
                        m_log.push_back("Inventory is at max capacity!");
                    }
                    else {
                        m_player.gold -= cost;
                        m_player.inventory.upgradeCapacity(10);
                        m_log.clear();
                        m_log.push_back("Inventory upgraded! (" + std::to_string(m_player.inventory.capacity()) + " slots)");
                    }
                }
                else {
                    // Leave
                    m_activeMerchant = nullptr;
                    m_state = GameState::Exploring;
                    m_log.clear();
                }
            }
            break;
        }

        case MerchantMode::Buy: {
            const auto& stock = m_activeMerchant->getStock();
            int   sel = m_activeMerchant->getSelectedIndex();
            if (action == Action::MoveUp)
                m_activeMerchant->setSelectedIndex(std::max(0, sel - 1));
            else if (action == Action::MoveDown)
                m_activeMerchant->setSelectedIndex(std::min((int)stock.size() - 1, sel + 1));
            else if (action == Action::Interact) {
                if (stock.empty()) return;
                const Item& item = stock[sel];
                int         price = m_activeMerchant->buyPrice(item);

                if (m_player.gold < price) {
                    m_log.clear();
                    m_log.push_back("Not enough gold! Need " + std::to_string(price) + "g");
                }
                else if (m_player.inventory.isFull()) {
                    m_log.clear();
                    m_log.push_back("Inventory full!");
                }
                else {
                    m_player.gold -= price;
                    m_player.inventory.addItem(item);
                    m_activeMerchant->removeStockItem(sel);
                    m_log.clear();
                    m_log.push_back("Bought: " + item.name + " (-" + std::to_string(price) + "g)");
                }
            }
            break;
        }

        case MerchantMode::Sell: {
            int invCount = m_player.inventory.count();
            if (invCount == 0) {
                m_merchantMode = MerchantMode::TopMenu;
                return;
            }

            if (action == Action::MoveUp)
                m_sellIndex = std::max(0, m_sellIndex - 1);
            else if (action == Action::MoveDown)
                m_sellIndex = std::min(invCount - 1, m_sellIndex + 1);
            else if (action == Action::Interact) {
                const Item& item = m_player.inventory.getItem(m_sellIndex);
                int         price = m_activeMerchant->sellPrice(item);

                m_player.gold += price;
                m_log.clear();
                m_log.push_back("Sold: " + item.name + " (+" + std::to_string(price) + "g)");
                m_player.inventory.removeItem(m_sellIndex);
                m_sellIndex = std::min(m_sellIndex, m_player.inventory.count() - 1);
                if (m_sellIndex < 0) m_sellIndex = 0;
            }
            break;
        }
        }
    }

    void Game::handleExitPrompt(Action action) {
        if (action == Action::Confirm) {
            nextFloor();
        }
        else if (action == Action::Deny || action == Action::Quit) {
            m_state = GameState::Exploring;
        }
    }

    void Game::nextFloor() {
        ++m_floor;
        m_dungeon.generate(m_floor);
        spawnPlayer();
        m_state = GameState::Exploring;
        m_inventoryMode = false;
        m_activeEnemy = nullptr;
        m_log.clear();
        m_log.push_back("You descend to floor " + std::to_string(m_floor) + ".");
    }

    void Game::handleQuitPrompt(Action action) {
        if (action == Action::Confirm) {
            m_running = false;  
        }
        else if (action == Action::Deny || action == Action::Quit) {
            m_state = GameState::Exploring;
        }
    }

    void Game::updateEnemyPatrol() {
    static std::mt19937 rng{ std::random_device{}() };

    const int dx[] = { 0, 0, -1, 1 };
    const int dy[] = { -1, 1, 0, 0 };

    for (auto& e : m_dungeon.getEntities()) {
        if (!e->isAlive()) continue;
        Enemy* enemy = dynamic_cast<Enemy*>(e.get());
        if (!enemy) continue;

        // collect valid moves
        std::vector<std::pair<int,int>> validMoves;
        for (int i = 0; i < 4; ++i) {
            int nx = enemy->getX() + dx[i];
            int ny = enemy->getY() + dy[i];

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
                    Merchant* m = dynamic_cast<Merchant*>(other.get());
                    if (m) { hasMerchant = true; break; }
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
            m_activeEnemy = enemy;
            m_state       = GameState::Combat;
            m_log.clear();
            m_log.push_back("-- " + enemy->getName() + " attacks you! --");
            m_log.push_back("Press Space to attack.");
            return; // stop all patrol movement, combat starts
        }

        enemy->setPosition(nx, ny);

    }
}
}