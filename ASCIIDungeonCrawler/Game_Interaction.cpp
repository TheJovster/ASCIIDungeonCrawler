#include "Game.h"
#include "AudioManager.h"

namespace DungeonGame 
{
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
                //play audio cue
                //play vfx for heal
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



    void Game::handleQuitPrompt(Action action) {
        if (action == Action::Confirm) {
            m_running = false;
        }
        else if (action == Action::Deny || action == Action::Quit) {
            m_state = GameState::Exploring;
        }
    }

}