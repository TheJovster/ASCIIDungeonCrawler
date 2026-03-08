#define NOMINMAX
#include "Game.h"
#include "InventoryItem.h"
#include "Merchant.h"
#include <windows.h>
#include <iostream>
#include <algorithm>
#include <random>

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

            // smooth rotation lerp
            float lerpSpeed = 10.f;
            float dt = clock.restart().asSeconds();
            if (dt > 0.1f)
                sf::err() << "Frame spike: " << dt << "s\n";
            float diff = m_player.targetAngle - m_player.angle;
            m_player.angle += diff * 10.f * dt;
            float posDiffX = (float)m_player.x - m_player.visualX;
            float posDiffY = (float)m_player.y - m_player.visualY;
            m_player.visualX += posDiffX * 10.f * dt;
            m_player.visualY += posDiffY * 10.f * dt;

            // lerp all entities toward their grid position
            for (auto& e : m_dungeon.getEntities()) {
                if (!e->isAlive()) continue;
                float edx = (float)e->getX() - e->visualX;
                float edy = (float)e->getY() - e->visualY;
                e->visualX += edx * 10.f * dt;
                e->visualY += edy * 10.f * dt;
                e->update(dt);
            }

            sf::Event event;
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
                }
                break;
            }

            window.clear(sf::Color::Black);
            m_raycastRenderer.draw(window, m_dungeon, m_player, dt);
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
                m_merchantMode, m_merchantTopSelected, m_sellIndex, combatData, m_time.getTimeString() + " " + m_time.getDayOfWeek() + " " + m_time.getDateString());
            window.display();
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
            m_player.targetAngle -= PI / 2.f;
            return;
        }
        if (action == Action::RotateRight) {
            m_player.targetAngle += PI / 2.f;
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
            newX = m_player.x + (int)std::round(std::cos(m_player.angle - PI / 2.f));
            newY = m_player.y + (int)std::round(std::sin(m_player.angle - PI / 2.f));
            break;
        case Action::MoveRight: // strafe right
            newX = m_player.x + (int)std::round(std::cos(m_player.angle + PI / 2.f));
            newY = m_player.y + (int)std::round(std::sin(m_player.angle + PI / 2.f));
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
        }
    }

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
            if (!e->isAlive() || !e->isMerchant()) continue;
            if (e->getX() == x && e->getY() == y)
                return static_cast<Merchant*>(e.get());
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

    int Game::getChestKeyAdjacent(int x, int y) const {
        int dx[] = { 0, 0, -1, 1 };
        int dy[] = { -1, 1, 0, 0 };
        for (int i = 0; i < 4; ++i) {
            int key = getChestKeyAt(x + dx[i], y + dy[i]);
            if (key != -1) return key;
        }
        return -1;
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

    void Game::handleGameOver(Action action) 
    {
        if (action == Action::Interact ||
            action == Action::Confirm ||
            action == Action::Quit) {
            m_running = false;
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

    m_pendingEnemyTurn = false;
    m_pendingCombatEnd = false;

    for (auto& e : m_dungeon.getEntities()) {
        if (!e->isAlive()) continue;
        if (!e->isEnemy()) continue;
        if (!e) continue;

        // collect valid moves
        std::vector<std::pair<int,int>> validMoves;
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
            m_player.targetAngle = std::atan2(dy, dx);
            m_state = GameState::Combat;
            m_combatPhase = CombatPhase::Resolution;
            m_combatActionSelected = 0;
            m_combat.clearLog();
            bool playerAlive = m_combat.enemyTurn(m_player, *m_activeEnemy);
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

    std::vector<int> Game::buildCombatItemList() {
        std::vector<int> result;
        for (int i = 0; i < m_player.inventory.count(); ++i) {
            if (m_player.inventory.getItem(i).type == ItemType::Consumable)
                result.push_back(i);
        }
        return result;
    }

    
}
